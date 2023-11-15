/*
 * File:   main.c
 * programmation de l'UART du PIC18F45K20 par interruption
 * Author: Jules et Benjamin
 * TP EN51: trop de code
 * Created on 13 septembre 2018, 08:51
 * Modifi� le 23/10/2023
 *  - D�tection message : <LF>OK<CR> idem \nOK\r\n
 *              trame compl�te : <CR><LF>OK<CR><LF>
 *  - D�tection message : CONNECT  "0012-6F-00C726"<CR>
 *              trame compl�te : <CR><LF>"0012-6F-00C726"<CR><LF>
 */

#include <xc.h>
#include "tmd_lib_v0.h"     //Biblioth�que de MD Tran
#include "configFuse.h"     //Configuration du PIC
#define _XTAL_FREQ 10000000 //Quartz 10 MHz utlis�

//D�claration des variables globales
bit flag_CONNECT=0;
bit flag_OK=0;
//Param�tres de gestion UART
#define buffin_size 60              // taille du buffer
char Data=0, Data_1=0, Data_2=0;    // initialisation tampon d�part 
char buffin[buffin_size];           // buffer de r�ception usart
int  rw_ptr = 0;                    // pointeur r�ception usart
int NbrInt=0;
short int TrameStart, TrameFin, TrameERROROK;// drapeaux de d�tection
char Start,Start1,Start2;           // caract�res d�but trame


//D�claration des prototypes
void Delay1Second(void);
void Delay200_ms(void);
void Delay100_ms(void);
void wait_OK();
void wait_CONNECT();

//Programme principale
void main(void)
{
    //d�claration des variables locales
    int i=0;    
    /* Initialisation Timer, ports et UART*/
    iniPorts();
    InitUSART_19200();

    /*Configuration de l'UART en interruption*/
    RCIF = 0; //reset RX pin flag
    RCIP = 0; //Not high priority
    RCIE = 1; //Enable RX interrupt
    PEIE = 1; //Enable pheripheral interrupt (serial port is a pheripheral)
    GIE = 1; //interruption g�n�rale

    for(i=0;i<2;i++)    //temporisation sans Timer
    {
        Delay1Second();
    }

    //1. Envoi de commande AT
    printf("at\r"); //Envoi de la commande "AT\r" pour test
    //2. Attente r�ponse "\nOK\r"
    wait_OK();      
    
    //suite de votre programme est ici
    //.........

    
    //boucle infinie pour test
    while(1){
        LATA4 = 0;      //la LED d6 est connect� sur le port RA4
        __delay_ms(20); //d�lai de 20 ms
        LATA4 = 1;
        Delay1Second(); //d�lai de 1s
    }
}


/* Fonction d'interruption */
void interrupt isr(void)
{
    int i=0; 
    if(PIR1bits.RCIF == 1) {    //interruption d�clench�e par l'UART
        PIR1bits.RCIF = 0;      //initialisation de la valeur du drapeau
        LATA4=~LATA4;           //inversion l'�tat de la Led pour test
        Data= RCREG;            //lecture de la valeur du buffer RX
        NbrInt++;               //incr�mentation du nombre de caract�res re�us

        //D�tection d�but trame
        //Exemple ( Data=='K' && Data_1=='O' && Data_2=='\n'))
        if( Data==Start2 && Data_1==Start1 && Data_2==Start) {
             TrameStart=1;
             buffin[1]=Data_2;
             buffin[2]=Data_1;
             rw_ptr=2;
             //debug : printf("debut Trame detectee\n");
            }
        
        if (TrameStart) {
             rw_ptr++;
             buffin[rw_ptr]=Data;
             //D�tection fin trame '\r' (Carriage Return)
             if( Data == '\r') {    
                 TrameStart=0;
                 TrameFin=1;
                 switch (rw_ptr) {

                    case 4: //\nOK\r    4 caract�res d�tect�s
                            flag_OK=1;
                    break;

                    case 26: //CONNECT  "0012-6F-00C726"\r soit 26 caract�res d�tect�s
                            flag_CONNECT=1;
                    break;

                    default: //erreur de d�tection
                            //printf("Error\r\n");
                            TrameERROROK=1;
                    break; 

                 Data=0;    
                 Data_1=0;
                 }//fin switch

                 //initialisation valeurs du buffer
                 for(i=0; i<=buffin_size ;i++){ 
                    buffin[i]=0;   //reset valeurs
                 }
                 rw_ptr=0;
           }//fin If if( Data == '\r')    
        }//fin If (TrameStart)
     Data_2=Data_1;
     Data_1=Data;
    }//fin de la routine
    
}

void Delay1Second()
{
    int i = 0;
    for(i=0;i<100;i++)
    {
         __delay_ms(10);
    }
}

void Delay200_ms()
{
    int i = 0;
    for(i=0;i<20;i++)   //200ms
    {
         __delay_ms(10);
    }
}

void Delay100_ms()
{
    int i = 0;
    for(i=0;i<10;i++)   //200ms
    {
         __delay_ms(10);
    }
}

void wait_OK()
{
    //attente r�ponse :"\r\nOK\r\n"
    Start='\n';
    Start1='O';
    Start2='K';
    flag_OK=0;
    LATA4 = 0;  //led allum�e
    while(!flag_OK){   //pr�voir une sortie de la boucle avec une variavle de retour!
    }
    flag_OK=0;
    LATA4 = 1;  //led �teinte, confirmation de r�ception
}

void wait_CONNECT(){
    //attente connexion avec r�ponse : CONNECT  "0012-6F-00C726"\r
    Start='C';
    Start1='O';
    Start2='N';
    flag_CONNECT=0;
    while(!flag_CONNECT){   //pr�voir une sortie de cette boucle
        flag_CONNECT=0;
        LATA4 = 0;
        Delay200_ms();
        LATA4 = 1;
        Delay200_ms();
     }

}



