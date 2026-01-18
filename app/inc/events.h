#ifndef INC_EVENTS_H_
#define INC_EVENTS_H_

//Define actuators
#define Distribution_Cartons_MSK	  			1 << (0+0)
#define Distribution_Cartons_OFF	  			0 << (0+0)
#define Distribution_Cartons_ON 	  			1 << (0+0)

#define Tapis_Distribution_Cartons_MSK 	 		1 << (1+0)
#define Tapis_Distribution_Cartons_OFF 	 		0 << (1+0)
#define Tapis_Distribution_Cartons_ON 	 		1 << (1+0)

#define Blocage_Entree_Palettiseur_MSK  	  	1 << (2+0)
#define Blocage_Entree_Palettiseur_OFF  	  	0 << (2+0)
#define Blocage_Entree_Palettiseur_ON  	  		1 << (2+0)

#define Porte_MSK 	  							1 << (3+0)
#define Porte_OFF 	  							0 << (3+0)
#define Porte_ON 	  							1 << (3+0)

#define Poussoir_MSK 	  						1 << (4+0)
#define Poussoir_OFF 	  						0 << (4+0)
#define Poussoir_ON 	  						1 << (4+0)

#define Clamp_MSK 	  							1 << (5+0)
#define Clamp_OFF 	  							0 << (5+0)
#define Clamp_ON 	  							1 << (5+0)

#define Monter_Ascenseur_MSK 	  				1 << (6+0)
#define Monter_Ascenseur_OFF 	  				0 << (6+0)
#define Monter_Ascenseur_ON 	  				1 << (6+0)

//#define skip one bit each 7 sensors or actuators

#define Descendre_Ascenseur_MSK 	  			1 << (7+1)
#define Descendre_Ascenseur_OFF 	  			0 << (7+1)
#define Descendre_Ascenseur_ON 	  				1 << (7+1)

#define Ascenceur_to_limit_MSK 	  				1 << (8+1)
#define Ascenceur_to_limit_OFF 	  				0 << (8+1)
#define Ascenceur_to_limit_ON 	  				1 << (8+1)

#define Distribution_Palette_MSK	  			1 << (9+1)
#define Distribution_Palette_OFF	  			0 << (9+1)
#define Distribution_Palette_ON	  				1 << (9+1)

#define Charger_Palette_MSK 	  				1 << (10+1)
#define Charger_Palette_OFF 	  				0 << (10+1)
#define Charger_Palette_ON 	  					1 << (10+1)

#define Tapis_Carton_vers_Palettiseur_MSK 	  	1 << (11+1)
#define Tapis_Carton_vers_Palettiseur_OFF 	  	0 << (11+1)
#define Tapis_Carton_vers_Palettiseur_ON 	  	1 << (11+1)

#define Tourner_Carton_MSK 	  					1 << (12+1)
#define Tourner_Carton_OFF 	  					0 << (12+1)
#define Tourner_Carton_ON 	  					1 << (12+1)

#define Decharger_Palettiseur_MSK 	  			1 << (13+1)
#define Decharger_Palettiseur_OFF 	  			0 << (13+1)
#define Decharger_Palettiseur_ON 	  			1 << (13+1)

//#define skip one bit each 7 sensors or actuators
#define Charger_Palettetiseur_MSK 	  			1 << (14+2)
#define Charger_Palettetiseur_OFF 	  			0 << (14+2)
#define Charger_Palettetiseur_ON 	  			1 << (14+2)


#define Decharger_Palette_MSK 	  				1 << (15+2)
#define Decharger_Palette_OFF 	  				0 << (15+2)
#define Decharger_Palette_ON 	  				1 << (15+2)

#define Tapis_Palette_Vers_Ascenseur_MSK 	  	1 << (16+2)
#define Tapis_Palette_Vers_Ascenseur_OFF 	  	0 << (16+2)
#define Tapis_Palette_Vers_Ascenseur_ON 	  	1 << (16+2)

#define Tapis_Distribution_Palette_MSK 	  		1 << (17+2)
#define Tapis_Distribution_Palette_OFF 	  		0 << (17+2)
#define Tapis_Distribution_Palette_ON 	  		1 << (17+2)

#define Tapis_Fin_MSK 	  						1 << (18+2)
#define Tapis_Fin_OFF 	  						0 << (18+2)
#define Tapis_Fin_ON 	  						1 << (18+2)

#define Remover_MSK 	  						1 << (19+2)
#define Remover_OFF 	  						0 << (19+2)
#define Remover_ON 	  							1 << (19+2)

//SENSOR STATE
#define ON 1
#define OFF 0

#define Carton_Distribue_MSK 	  				1<< (0+0)
#define Carton_Distribue_OFF 	  				0
#define Carton_Distribue_ON 	  				1

#define Carton_Envoye_MSK 	  					1 << (1+0)
#define Carton_Envoye_OFF 	  					0
#define Carton_Envoye_ON 	  					1

#define Entree_Palettiseur_MSK  				1 << (2+0)
#define Entree_Palettiseur_OFF  				0
#define Entree_Palettiseur_ON  					1

#define Porte_Ouverte_MSK 	  					1 << (3+0)
#define Porte_Ouverte_OFF 	  					0
#define Porte_Ouverte_ON 	  					1

#define Limite_Poussoir_MSK 	  				1 << (4+0)
#define Limite_Poussoir_OFF 	  				0
#define Limite_Poussoir_ON 	  					1

#define Clamped_MSK 	  						1 << (5+0)
#define Clamped_OFF 	  						0
#define Clamped_ON 	  							1

#define Ascenceur_Etage_RDC_MSK 	  			1 << (6+0)
#define Ascenceur_Etage_RDC_OFF 	  			0
#define Ascenceur_Etage_RDC_ON 	  				1

//#define skip one bit each 7 sensors or actuators
#define Ascenceur_Etage_1_MSK 	 				1 << (7+1)
#define Ascenceur_Etage_1_OFF 	 				0
#define Ascenceur_Etage_1_ON 	 				1

#define Ascenceur_Etage_2_MSK	    			1 << (8+1)
#define Ascenceur_Etage_2_OFF	    			0
#define Ascenceur_Etage_2_ON	    			1

#define Sortie_Palette_MSK    					1 << (9+1)
#define Sortie_Palette_OFF    					0
#define Sortie_Palette_ON    					1

#define Limite_Porte_MSK    					1 << (10+1)
#define Limite_Porte_OFF    					0
#define Limite_Porte_ON    						1

#define Ascenceur_en_mouvement_MSK 				1 << (11+1)
#define Ascenceur_en_mouvement_OFF    			0
#define Ascenceur_en_mouvement_ON    			1

#define Entree_Palette_MSK		   				1 << (12+1)
#define Entree_Palette_OFF    					0
#define Entree_Palette_ON    					1

#define Butee_Carton_MSK    					1 << (13+1)
#define Butee_Carton_OFF						0
#define Butee_Carton_ON    						1

#endif /* INC_EVENTS_H_ */
