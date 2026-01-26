#include "main.h"

// Queue
xQueueHandle xComQueue;
xQueueHandle xSubcribeQueue;

// Semaphores
xSemaphoreHandle xSem_UART_TC, xSem_DMA_TC;
xSemaphoreHandle xSem1, xSem2, xSem3;
xSemaphoreHandle xSemCarton;
xSemaphoreHandle xSemAscenseur;
xSemaphoreHandle xSemPalette;
xSemaphoreHandle xSemPoussoirEvt;
xSemaphoreHandle xSemPush2;
xSemaphoreHandle xSem6Boxes;
xSemaphoreHandle xSemPorteEvt;
xSemaphoreHandle xConsoleMutex;

// Tasks
void vTaskPalette(void *pvParameters);
void vTaskRead(void *pvParameters);
void vTaskWrite(void *pvParameters);
void vTaskAscenseur(void *pvParameters);
void vTaskBoxGenerator(void *pvParameters);
void vTaskGateAndPusher(void *pvParameters);
void vTaskPoussoir2Boxes(void *pvParameters);

// Global variables
extern uint8_t tx_dma_buffer[7];
extern uint8_t rx_dma_buffer[FRAME_LENGTH];
uint8_t sensor[SENSOR_TABLE_SIZE];

// Static functions
static void SystemClock_Config(void);
static inline void SEND_CMD(uint32_t mask, uint32_t state);

// Main program
int main(void)
{
    // Configure System Clock
    SystemClock_Config();

    // Initialize LED pin
    BSP_LED_Init();

    // Initialize Debug Console
    BSP_Console_Init();

    // Start Trace Recording
    vTraceEnable(TRC_START);

    // Create Semaphore objects
    xSem_UART_TC = xSemaphoreCreateBinary();
    xSem_DMA_TC = xSemaphoreCreateBinary();
    xSem1 = xSemaphoreCreateBinary();
    xSem2 = xSemaphoreCreateBinary();
    xSem3 = xSemaphoreCreateBinary();
    xSemCarton = xSemaphoreCreateBinary();
    xSemPalette = xSemaphoreCreateBinary();
    xSemAscenseur = xSemaphoreCreateBinary();
    xSemPush2 = xSemaphoreCreateBinary();
    xSemPoussoirEvt = xSemaphoreCreateBinary();
    xSem6Boxes = xSemaphoreCreateBinary();
    xSemPorteEvt = xSemaphoreCreateBinary();

    // Create a Mutex for accessing the console
    xConsoleMutex = xSemaphoreCreateMutex();

    // Create Queue to hold console messages
    xComQueue = xQueueCreate(20, sizeof(command_message_t));
    xSubcribeQueue = xQueueCreate(10, sizeof(subscribe_message_t*));

    command_message_t command;
    command.state = 0;
    command.mask  = 0;

    command.mask =
        Tapis_Distribution_Cartons_MSK |
        Blocage_Entree_Palettiseur_MSK |
        Tapis_Carton_vers_Palettiseur_MSK |
        Tapis_Palette_Vers_Ascenseur_MSK |
        Tapis_Distribution_Palette_MSK |
        Tapis_Fin_MSK |
        Remover_MSK;
        //|Distribution_Cartons_MSK;

    // TURN ON THE CONCERNED BITS
    command.state =
        Tapis_Distribution_Cartons_ON |
        Blocage_Entree_Palettiseur_ON |
        Tapis_Carton_vers_Palettiseur_ON |
        Tapis_Palette_Vers_Ascenseur_ON |
        Tapis_Distribution_Palette_ON |
        Tapis_Fin_ON |
        Remover_ON;
        //|Distribution_Cartons_ON;

    // Send message to the Console Queue
    xQueueSendToBack(xComQueue, &command, 0);

    xTaskCreate(vTaskPalette, "vTaskPalette", 96, NULL, 2, NULL);
    xTaskCreate(vTaskAscenseur, "vTaskAscenseur", 128, NULL, 4, NULL);
    xTaskCreate(vTaskRead, "vTaskRead", 192, NULL, 6, NULL);
    xTaskCreate(vTaskWrite, "vTaskWrite", 192, NULL, 5, NULL);
    xTaskCreate(vTaskBoxGenerator, "BoxGen", 96, NULL, 4, NULL);
    xTaskCreate(vTaskGateAndPusher, "GatePush", 192, NULL, 4, NULL);
    xTaskCreate(vTaskPoussoir2Boxes, "Push2", 192, NULL, 4, NULL);

    // Start the Scheduler
    vTaskStartScheduler();

    while (1)
    {
        // The program should never be here...
    }
}

static inline void SEND_CMD(uint32_t mask, uint32_t state)
{
    command_message_t cmd;
    cmd.mask = mask;
    cmd.state = state;
    xQueueSendToBack(xComQueue, &cmd, 0);
}

void vTaskBoxGenerator(void *pvParameters)
{
    const TickType_t PERIOD = 10000 / portTICK_RATE_MS; // 10s
    const TickType_t PULSE = 250 / portTICK_RATE_MS; // 250ms

    TickType_t last = xTaskGetTickCount();

    while (1)
    {
        SEND_CMD(Distribution_Cartons_MSK, Distribution_Cartons_ON);
        vTaskDelay(PULSE);
        SEND_CMD(Distribution_Cartons_MSK, Distribution_Cartons_OFF);

        vTaskDelayUntil(&last, PERIOD);
    }
}

void vTaskGateAndPusher(void *pvParameters)
{
    subscribe_message_t sub;
    subscribe_message_t *ps = &sub;

    uint8_t state = 0; // 0=espera 1a caixa, 1=espera 2a caixa

    SEND_CMD(Blocage_Entree_Palettiseur_MSK, Blocage_Entree_Palettiseur_ON);
    SEND_CMD(Charger_Palettetiseur_MSK,      Charger_Palettetiseur_OFF);
    SEND_CMD(Poussoir_MSK,                   Poussoir_OFF);

    while (1)
    {
        // 1) espera 1 caixa gerar um pulso completo no Entree (ON depois OFF)
        sub.sem_id = 1;
        sub.sensor_id = Entree_Palettiseur_MSK;
        sub.sensor_state = Entree_Palettiseur_ON;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemCarton, portMAX_DELAY);

        sub.sem_id = 1;
        sub.sensor_id = Entree_Palettiseur_MSK;
        sub.sensor_state = Entree_Palettiseur_OFF;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemCarton, portMAX_DELAY);

        // 2) pares abrem, ímpares fecham
        if (state == 0)
		{
			// Chegou a 1a do par: mantém fechado (segura a próxima)
			SEND_CMD(Blocage_Entree_Palettiseur_MSK, Blocage_Entree_Palettiseur_ON);
			SEND_CMD(Charger_Palettetiseur_MSK, Charger_Palettetiseur_OFF);
			state = 1;
		}
		else
		{
			// Chegou a 2a do par: abre para entrarem as DUAS e puxa
			SEND_CMD(Blocage_Entree_Palettiseur_MSK, Blocage_Entree_Palettiseur_OFF);
			SEND_CMD(Charger_Palettetiseur_MSK, Charger_Palettetiseur_ON);

			vTaskDelay(4000);

			// E já volta a fechar para não deixar a 3a entrar antes de empurrar
			SEND_CMD(Blocage_Entree_Palettiseur_MSK, Blocage_Entree_Palettiseur_ON);
			SEND_CMD(Charger_Palettetiseur_MSK, Charger_Palettetiseur_OFF);

			// Agora sinaliza o empurrador
			xSemaphoreGive(xSemPush2);

			state = 0;
		}

        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

void vTaskPoussoir2Boxes(void *pvParameters)
{
    const TickType_t SMALL_WAIT = 30 / portTICK_RATE_MS;

    subscribe_message_t sub;
    subscribe_message_t *ps = &sub;

    uint8_t pushes = 0; // 1 empurrão = 2 caixas -> 3 = 6 caixas

    SEND_CMD(Poussoir_MSK, Poussoir_OFF);

    while (1)
    {
        xSemaphoreTake(xSemPush2, portMAX_DELAY);

        // home (ON)
        sub.sem_id = 4;
        sub.sensor_id = Limite_Poussoir_MSK;
        sub.sensor_state = Limite_Poussoir_ON;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemPoussoirEvt, portMAX_DELAY);

        // avança
        SEND_CMD(Poussoir_MSK, Poussoir_ON);

        // sai do home (OFF)
        sub.sem_id = 4;
        sub.sensor_id = Limite_Poussoir_MSK;
        sub.sensor_state = Limite_Poussoir_OFF;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemPoussoirEvt, portMAX_DELAY);

        // volta ao home (ON)
        sub.sem_id = 4;
        sub.sensor_id = Limite_Poussoir_MSK;
        sub.sensor_state = Limite_Poussoir_ON;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemPoussoirEvt, portMAX_DELAY);

        // recolhe
        SEND_CMD(Poussoir_MSK, Poussoir_OFF);
        vTaskDelay(SMALL_WAIT);

        pushes++;
        if (pushes >= 3)
        {
            pushes = 0;
            xSemaphoreGive(xSem3);   // <-- DISPARA O CICLO DO ELEVADOR (E ELE ABRE A PORTE)
        }
    }
}

void vTaskAscenseur(void *pvParameters)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    subscribe_message_t s;
    subscribe_message_t *ps = &s;

    command_message_t command;
    command.state = 0;
    command.mask  = 0;

    // Tempos (ajusta se precisar)
    const TickType_t SETTLE_WAIT = 200 / portTICK_RATE_MS;
    const TickType_t CLAMP_HOLD = 250 / portTICK_RATE_MS;
    const TickType_t DROP_WAIT = 1200 / portTICK_RATE_MS; // tempo pra cair após abrir
    const TickType_t NUDGE_DOWN_TIME = 350 / portTICK_RATE_MS;  // descer um pouco antes de fechar (se precisar)

    while (1)
    {
        // =========================================================
        // LOTE 1: espera 6 caixas chegarem EM CIMA DO PORTE FECHADO
        // =========================================================
        xSemaphoreTake(xSem3, portMAX_DELAY);

        // ---------------------------------------------------------
        // 1) SUBIR ATÉ ETAGE 1 (com limite)
        // ---------------------------------------------------------
        command.mask  = (Monter_Ascenseur_MSK | Ascenceur_to_limit_MSK);
        command.state = (Monter_Ascenseur_ON  | Ascenceur_to_limit_ON);
        xQueueSendToBack(xComQueue, &command, 0);

        if (FACTORY_IO_Sensors_Get(Ascenceur_Etage_1_MSK) != Ascenceur_Etage_1_ON)
        {
            s.sem_id = 3;
            s.sensor_id = Ascenceur_Etage_1_MSK;
            s.sensor_state = Ascenceur_Etage_1_ON;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        // ---------------------------------------------------------
        // 2) CLAMP ON (ANTES DE ABRIR O PORTE)
        // ---------------------------------------------------------
        command.mask = Clamp_MSK;
        command.state = Clamp_ON;
        xQueueSendToBack(xComQueue, &command, 0);

        if (FACTORY_IO_Sensors_Get(Clamped_MSK) != Clamped_ON)
        {
            s.sem_id = 3;
            s.sensor_id = Clamped_MSK;
            s.sensor_state = Clamped_ON;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        vTaskDelay(CLAMP_HOLD);

        // ---------------------------------------------------------
        // 3) ABRIR PORTE e parar subida/limite (como ele faz junto)
        // ---------------------------------------------------------
        command.mask  = (Porte_MSK | Ascenceur_to_limit_MSK | Monter_Ascenseur_MSK);
        command.state = (Porte_ON | Ascenceur_to_limit_OFF | Monter_Ascenseur_OFF);
        xQueueSendToBack(xComQueue, &command, 0);

        if (FACTORY_IO_Sensors_Get(Porte_Ouverte_MSK) != Porte_Ouverte_ON)
        {
            s.sem_id = 3;
            s.sensor_id = Porte_Ouverte_MSK;
            s.sensor_state = Porte_Ouverte_ON;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        if (FACTORY_IO_Sensors_Get(Limite_Porte_MSK) != Limite_Porte_OFF)
        {
            s.sem_id = 3;
            s.sensor_id = Limite_Porte_MSK;
            s.sensor_state = Limite_Porte_OFF;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        // deixa cair
        vTaskDelay(DROP_WAIT);

        // ---------------------------------------------------------
        // 4) DESCE UM POUCO (teu detalhe do “dar espaço pro clamp/porte”)
        // ---------------------------------------------------------
        command.mask = Descendre_Ascenseur_MSK;
        command.state = Descendre_Ascenseur_ON;
        xQueueSendToBack(xComQueue, &command, 0);

        // espera sair do Etage 1
        if (FACTORY_IO_Sensors_Get(Ascenceur_Etage_1_MSK) != Ascenceur_Etage_1_OFF)
        {
            s.sem_id = 3;
            s.sensor_id = Ascenceur_Etage_1_MSK;
            s.sensor_state = Ascenceur_Etage_1_OFF;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        // desce mais um tiquinho (pra garantir folga pro porte fechar)
        vTaskDelay(NUDGE_DOWN_TIME);

        // para de descer por enquanto
        command.mask = Descendre_Ascenseur_MSK;
        command.state = Descendre_Ascenseur_OFF;
        xQueueSendToBack(xComQueue, &command, 0);

        vTaskDelay(SETTLE_WAIT);

        // ---------------------------------------------------------
        // 5) FECHA PORTE + CLAMP OFF
        // ---------------------------------------------------------
        command.mask = (Clamp_MSK | Porte_MSK);
        command.state = (Clamp_OFF | Porte_OFF);
        xQueueSendToBack(xComQueue, &command, 0);

        // (opcional, mas ajuda a não “bugar”)
        if (FACTORY_IO_Sensors_Get(Limite_Porte_MSK) != Limite_Porte_ON)
        {
            s.sem_id = 3;
            s.sensor_id = Limite_Porte_MSK;
            s.sensor_state = Limite_Porte_ON;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        vTaskDelay(SETTLE_WAIT);

        // =========================================================
        // LOTE 2: espera 6 caixas chegarem EM CIMA DO PORTE FECHADO
        // =========================================================
        xSemaphoreTake(xSem3, portMAX_DELAY);

        // ---------------------------------------------------------
        // 6) CLAMP ON + PORTE ON juntos
        // ---------------------------------------------------------
        command.mask = (Clamp_MSK | Porte_MSK);
        command.state = (Clamp_ON  | Porte_ON);
        xQueueSendToBack(xComQueue, &command, 0);

        // espera porta aberta
        if (FACTORY_IO_Sensors_Get(Porte_Ouverte_MSK) != Porte_Ouverte_ON)
        {
            s.sem_id = 3;
            s.sensor_id = Porte_Ouverte_MSK;
            s.sensor_state = Porte_Ouverte_ON;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        // espera sair do limite fechado
        if (FACTORY_IO_Sensors_Get(Limite_Porte_MSK) != Limite_Porte_OFF)
        {
            s.sem_id = 3;
            s.sensor_id = Limite_Porte_MSK;
            s.sensor_state = Limite_Porte_OFF;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        // deixa cair segunda camada
        vTaskDelay(DROP_WAIT);

        // ---------------------------------------------------------
        // 7) DESCE ATÉ RDC COM LIMITE
        // ---------------------------------------------------------
        command.mask = (Descendre_Ascenseur_MSK | Ascenceur_to_limit_MSK);
        command.state = (Descendre_Ascenseur_ON  | Ascenceur_to_limit_ON);
        xQueueSendToBack(xComQueue, &command, 0);

        if (FACTORY_IO_Sensors_Get(Ascenceur_Etage_RDC_MSK) != Ascenceur_Etage_RDC_ON)
        {
            s.sem_id = 3;
            s.sensor_id = Ascenceur_Etage_RDC_MSK;
            s.sensor_state = Ascenceur_Etage_RDC_ON;
            xQueueSendToBack(xSubcribeQueue, &ps, 0);
            xSemaphoreTake(xSemAscenseur, portMAX_DELAY);
        }

        // ---------------------------------------------------------
        // 8) FECHA PORTE + para elevador + clamp OFF (final)
        // ---------------------------------------------------------
        command.mask = (Ascenceur_to_limit_MSK | Porte_MSK | Descendre_Ascenseur_MSK | Clamp_MSK);
        command.state = (Ascenceur_to_limit_OFF | Porte_OFF | Descendre_Ascenseur_OFF | Clamp_OFF);
        xQueueSendToBack(xComQueue, &command, 0);

        // libera a task da palette levar até o fim
        xSemaphoreGive(xSem2);

        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_RATE_MS);
    }
}


void vTaskPalette(void *pvParameters)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    // VARIABLES
    subscribe_message_t mPalette;
    subscribe_message_t *ps;

    command_message_t *m_cmd;
    command_message_t command;

    command.state = 0;
    command.mask  = 0;

    while (1)
    {
        // Espera o palete sair
        mPalette.sem_id = 2;
        mPalette.sensor_id = Sortie_Palette_MSK;
        mPalette.sensor_state = Sortie_Palette_OFF;
        ps = &mPalette;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);

        // Semaforo de sincronização
        xSemaphoreTake(xSemPalette, portMAX_DELAY);

        // Liga a distribuição do palete
        command.mask = Distribution_Palette_MSK;
        command.state = Distribution_Palette_ON;
        m_cmd = &command;
        (void)m_cmd;
        xQueueSendToBack(xComQueue, &command, 0);

        // Esperca a distribução
        vTaskDelay(500);

        // Desliga a distribuição do palete
        command.mask = Distribution_Palette_MSK;
        command.state = Distribution_Palette_OFF;
        xQueueSendToBack(xComQueue, &command, 0);

        // Espera o palete chegar
        mPalette.sem_id = 2;
        mPalette.sensor_id = Entree_Palette_MSK;
        mPalette.sensor_state = Entree_Palette_ON;
        ps = &mPalette;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemPalette, portMAX_DELAY);

        // Liga o palete loading conveyor
        command.mask  = Charger_Palette_MSK;
        command.state = Charger_Palette_ON;
        xQueueSendToBack(xComQueue, &command, 0);

        // Espera o palete ir pra saida
        mPalette.sem_id = 2;
        mPalette.sensor_id = Sortie_Palette_MSK;
        mPalette.sensor_state = Sortie_Palette_ON;
        ps = &mPalette;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemPalette, portMAX_DELAY);

        // Desliga o palete loading conveyor
        command.mask = Charger_Palette_MSK;
        command.state = Charger_Palette_OFF;
        xQueueSendToBack(xComQueue, &command, 0);

        // Semaforo de sincronização
        xSemaphoreTake(xSem2, portMAX_DELAY);

        // Liga o palete loading conveyor
        command.mask = Charger_Palette_MSK;
        command.state = Charger_Palette_ON;
        xQueueSendToBack(xComQueue, &command, 0);

        // Espera o palete ir pra saida
        mPalette.sem_id = 2;
        mPalette.sensor_id = Sortie_Palette_MSK;
        mPalette.sensor_state = Sortie_Palette_OFF;
        ps = &mPalette;
        xQueueSendToBack(xSubcribeQueue, &ps, 0);
        xSemaphoreTake(xSemPalette, portMAX_DELAY);

        // Desliga o palete loading conveyor
        command.mask = Charger_Palette_MSK;
        command.state = Charger_Palette_OFF;
        xQueueSendToBack(xComQueue, &command, 0);

        vTaskDelayUntil(&xLastWakeTime, 200 / portTICK_RATE_MS); // 200 ms wait
    }
}

void vTaskRead(void *pvParameters)
{
    // expected type
    subscribe_message_t *s_message;

    // array
    subscribe_message_t tab[SIZE];

    portBASE_TYPE xStatus;
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    uint8_t existe = 0;

    for (int i = 0; i < SIZE; i++)
    {
        tab[i].sem_id = 0;
        tab[i].sensor_id = 0;
        tab[i].sensor_state = 0;
    }

    xSemaphoreGive(xSem1);

    while (1)
    {
        xStatus = xQueueReceive(xSubcribeQueue, &s_message, 0);

        if (xStatus == pdPASS)
        {
            // Checking duplicates
            for (int i = 0; i < SIZE; i++)
            {
                if ((s_message->sem_id == tab[i].sem_id) &&
                    (s_message->sensor_id == tab[i].sensor_id) &&
                    (s_message->sensor_state == tab[i].sensor_state))
                {
                    existe = 1;
                }
            }

            // Fill first empty slot
            for (int i = 0; i < SIZE; i++)
            {
                if ((tab[i].sem_id == 0) && (existe == 0))
                {
                    tab[i].sem_id       = s_message->sem_id;
                    tab[i].sensor_id    = s_message->sensor_id;
                    tab[i].sensor_state = s_message->sensor_state;
                    existe = 1;
                }
            }

            existe = 0;
        }

        xSemaphoreTake(xConsoleMutex, portMAX_DELAY);

        for (int i = 0; i < SIZE; i++)
        {
            if (tab[i].sem_id != 0)
            {
                if (FACTORY_IO_Sensors_Get(tab[i].sensor_id) == tab[i].sensor_state)
                {
                    if (tab[i].sem_id == 1)
                    {
                        xSemaphoreGive(xSemCarton);

                        tab[i].sem_id = 0;
                        tab[i].sensor_id = 0;
                        tab[i].sensor_state = 0;
                    }

                    if (tab[i].sem_id == 2)
                    {
                        xSemaphoreGive(xSemPalette);

                        tab[i].sem_id = 0;
                        tab[i].sensor_id = 0;
                        tab[i].sensor_state = 0;
                    }

                    if (tab[i].sem_id == 3)
                    {
                        xSemaphoreGive(xSemAscenseur);

                        tab[i].sem_id = 0;
                        tab[i].sensor_id = 0;
                        tab[i].sensor_state = 0;
                    }

                    if (tab[i].sem_id == 4)
                    {
                        xSemaphoreGive(xSemPoussoirEvt);

                        tab[i].sem_id = 0;
                        tab[i].sensor_id = 0;
                        tab[i].sensor_state = 0;
                    }

                    if (tab[i].sem_id == 5)
                    {
                        xSemaphoreGive(xSemPorteEvt);

                        tab[i].sem_id = 0;
                        tab[i].sensor_id = 0;
                        tab[i].sensor_state = 0;
                    }
                }
            }
        }

        xSemaphoreGive(xConsoleMutex);

        vTaskDelayUntil(&xLastWakeTime, 200 / portTICK_RATE_MS);
    }
}

void vTaskWrite(void *pvParameters)
{
    uint32_t memoire = 0;
    command_message_t cmd;

    portBASE_TYPE xStatus;
    command_message_t c_message;

    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    // Set maximum priority for DMA interrupts
    NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

    while (1)
    {
        xStatus = xQueueReceive(xComQueue, &c_message, portMAX_DELAY);

        xSemaphoreTake(xConsoleMutex, portMAX_DELAY);

        if (xStatus == pdPASS)
        {
            cmd = c_message;

            memoire &= ~cmd.mask;
            memoire |=  cmd.state;

            // Prepare frame buffer
            tx_dma_buffer[0] = TAG_ACTUATORS;
            tx_dma_buffer[1] = (uint8_t) (memoire & 0x000000FF);
            tx_dma_buffer[2] = (uint8_t)((memoire & 0x0000FF00) >> 8U);
            tx_dma_buffer[3] = (uint8_t)((memoire & 0x00FF0000) >> 16U);
            tx_dma_buffer[4] = (uint8_t)((memoire & 0xFF000000) >> 24U);
            tx_dma_buffer[5] = 0x00;  // CRC (not yet implemented)
            tx_dma_buffer[6] = '\n';  // End byte

            // Set Memory Buffer size
            DMA1_Channel4->CNDTR = 7;

            // Enable DMA1 Channel 4
            DMA1_Channel4->CCR |= DMA_CCR_EN;

            // Enable USART2 DMA Request on Tx
            USART2->CR3 |= USART_CR3_DMAT;

            // Wait for Semaphore endlessly
            xSemaphoreTake(xSem_DMA_TC, portMAX_DELAY);

            // Disable USART2 DMA Request on Tx
            USART2->CR3 &= ~USART_CR3_DMAT;

            // Disable DMA1 Channel 4
            DMA1_Channel4->CCR &= ~DMA_CCR_EN;
        }

        xSemaphoreGive(xConsoleMutex);

        vTaskDelayUntil(&xLastWakeTime, 200 / portTICK_RATE_MS);
    }
}

static void SystemClock_Config(void)
{
    uint32_t HSE_Status;
    uint32_t PLL_Status;
    uint32_t SW_Status;
    uint32_t timeout = 0;

    timeout = 1000000;

    // Start HSE in Bypass Mode
    RCC->CR |= RCC_CR_HSEBYP;
    RCC->CR |= RCC_CR_HSEON;

    // Wait until HSE is ready
    do {
        HSE_Status = RCC->CR & RCC_CR_HSERDY_Msk;
        timeout--;
    } while ((HSE_Status == 0) && (timeout > 0));

    // Select HSE as PLL input source
    RCC->CFGR &= ~RCC_CFGR_PLLSRC_Msk;
    RCC->CFGR |= (0x02 << RCC_CFGR_PLLSRC_Pos);

    // Set PLL PREDIV to /1
    RCC->CFGR2 = 0x00000000;

    // Set PLL MUL to x6
    RCC->CFGR &= ~RCC_CFGR_PLLMUL_Msk;
    RCC->CFGR |= (0x04 << RCC_CFGR_PLLMUL_Pos);

    // Enable the main PLL
    RCC->CR |= RCC_CR_PLLON;

    // Wait until PLL is ready
    do {
        PLL_Status = RCC->CR & RCC_CR_PLLRDY_Msk;
        timeout--;
    } while ((PLL_Status == 0) && (timeout > 0));

    // Set AHB prescaler to /1
    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    // Set APB1 prescaler to /1
    RCC->CFGR &= ~RCC_CFGR_PPRE_Msk;
    RCC->CFGR |= RCC_CFGR_PPRE_DIV1;

    // Enable FLASH Prefetch Buffer and set Flash Latency
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    /* --- Switching to PLL at 48MHz Now! --- */

    // Select the main PLL as system clock source
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    // Wait until PLL becomes main switch input
    do {
        SW_Status = (RCC->CFGR & RCC_CFGR_SWS_Msk);
        timeout--;
    } while ((SW_Status != RCC_CFGR_SWS_PLL) && (timeout > 0));

    /*--- Use PA8 as LSE output ---*/
    RCC->CFGR &= ~RCC_CFGR_MCO_Msk;
    RCC->CFGR |= RCC_CFGR_MCOSEL_LSE;

    // No prescaler
    RCC->CFGR &= ~RCC_CFGR_MCOPRE_Msk;

    // Enable GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Configure PA8 as Alternate function
    GPIOA->MODER &= ~GPIO_MODER_MODER8_Msk;
    GPIOA->MODER |= (0x02 << GPIO_MODER_MODER8_Pos);

    // Set to AF0 (MCO output)
    GPIOA->AFR[1] &= ~(0x0000000F);
    GPIOA->AFR[1] |= (0x00000000);

    // Update SystemCoreClock global variable
    SystemCoreClockUpdate();
}
