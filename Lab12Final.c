/*******************************************************************************
*                           	    UTFPR
*
*  							PIC16F877	Compilador XC8
*
********************************************************************************
********************************************************************************
*                     	    DESCRIÇÃO DO ARQUIVO                        
* Aluno: Juliano Rodrigues Dourado	- 	RA: 1307371
* Projeto: Calculadora - Laboratório 12
* Data: 25/05/2017
*
********************************************************************************
*              	CONFIGURAÇÃO PARA GRAVAÇÃO (FUSES)
*******************************************************************************/
#pragma config BOREN = ON   //BROWN-OUT RESET ativado.
#pragma config CPD = OFF    //Proteção de dados da EEPROM desligada.
#pragma config DEBUG = OFF  //In-Circuit Debugger desligado
#pragma config WRT = OFF    //Proteção da Flash desativada.
#pragma config FOSC = HS    //Seleção do oscilador ( > 4MHZ ).
#pragma config WDTE = OFF   //Watchdog desativado.
#pragma config CP = OFF     //Code Protection.
#pragma config LVP = OFF    //Programação em Low-Voltage In-Circuit Serial.
#pragma config PWRTE = ON   //POWER-UP TIMER ativado.

/*******************************************************************************
*                            CABEÇALHOS INCLUIDOS
*******************************************************************************/
#include <xc.h>     		//Diretiva de compilação para o XC8.
#include <stdio.h>			//Diretiva para a biblioteca padrão STDIO
#include <stdlib.h>			//Diretiva para a biblioteca padrão STDILIB
#include <string.h>			//Diretiva para a biblioteca padrão STRING


/*******************************************************************************
*                                  CONSTANTES
*******************************************************************************/
#define _XTAL_FREQ  16000000    //Definição do clock (Hz).

/*******************************************************************************
*                               VARIÁVEIS GLOBAIS
*******************************************************************************/

signed short long Operando2	= 0;	
signed short long Operando1	= 0;
float resultado = 0;
char Status1 = 0b11111111, Atpl1 = 0b11111111, Pl1 = 0b11111111, Ul1 = 0b11111111, Status2 = 0b11111111, Atpl2 = 0b11111111, Pl2 = 0b11111111, Ul2 = 0b11111111;	//Cria as variáveis necessárias para guardar e efetuar as leituras do teclado, 
char Status3 = 0b11111111, Atpl3 = 0b11111111, Pl3 = 0b11111111, Ul3 = 0b11111111, Status4 = 0b11111111, Atpl4 = 0b11111111, Pl4 = 0b11111111, Ul4 = 0b11111111; 	//para o processo de debouncing.
int Tvarredura = 0;	
char valor = 255 , sinal1, sinal2, UDC = 0,  sinalOp;
int EstadoAtual = 1, linha2 = 0, EstadoSecundario = 1;
int PerderTempo = 0;
char res[30];
char aux3[30];
int DeslocaCursor = 1;
int cont, i, j,num, FLimpDisp = 0, OpCompleta = 0, back = 0, back1 = 0, back2 = 0, k, increment = 0, acumula;
char saida1, saida2;
unsigned long result, armazena, acumula1, aux1;

/*******************************************************************************
*                           PROTÓTIPOS DAS FUNÇÕES
*******************************************************************************/
void inic_regs(void);			//Protótipo da função que configura os registradores especiais de acordo com o desejado.
void inic_disp(void);			//Protótipo da função que inicializa o display.
void Estado1(void);				//Protótipo da função que é o Estado 1 da máquina principal, onde ocorre a varredura do teclado.
void Estado2(void);				//Protótipo da função que é o Estado 2 da máquina principal, onde ocorre a escrita no display conforme desejado.
void Estado3(void);				//Protótipo da função que é o Estado 3 da maquina principal, onde realiza-se a operação de acordo com o desejado.	
void Estado31(void);			//Protótipo da função que é o primeiro subestado do Estado3, que receberá o sinal do primeiro operando.
void Estado32(void);            //Protótipo da função que é o segundo subestado do Estado3, que atualizará o valor propriamente dito do primeiro operando.
void Estado33(void);			//Protótipo da função que é o terceiro subestado do Estado3, que atualizará o sinal do segundo Operando.
void Estado34(void);			//Protótipo da função que é o quarto subestado do Estado3, que realizará a operação entre os operandos conforme digitado.

/*******************************************************************************
* interrupt isr
********************************************************************************
* Descrição:			Interrupção do microcontrolador. Não usada.
* Autor:					
* Data de Criação:		
* Entrada:				
* Saída:				
*******************************************************************************/
void interrupt isr (void)
{

}

/*******************************************************************************
* main
********************************************************************************
* Descrição:    Função principal, com chamada das configurações e loop.
* Autor:	    Juliano Rodrigues Dourado
* Data de Criação:	25/05/2017
* Entrada: Nenhuma.
* Saída: Nenhuma.
*******************************************************************************/

void main(void){
	
	inic_regs();	//Inicializa os registradores especiais, realizando as configurações confome desejado.
	inic_disp();	//Inicializa o display.
    
	while(1){		//loop principal
		
		if(EstadoAtual == 1){		
			Estado1();			//Realiza a varredura do teclado.
		}
		else if(EstadoAtual == 2){
			Estado2();			//Mostra no display o correspondente valor digitado no teclado, ou o resultado da operação.
		}
		else if(EstadoAtual == 3){
			Estado3();			//Realiza a operação e atualização dos Operandos conforme digitado pelo usuário.
		}
    }
	
}

/*******************************************************************************
* inic_regs
********************************************************************************
* Descrição:	Configura os registradores especiais e os periféricos.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	25/05/2017
* Entrada:		Nenhuma
* Saída:		Nenhuma
*******************************************************************************/
void inic_regs(void)
{
    TRISA = 0b00011111;			//Define o bit/pino RA5 como saída. 	
    TRISC = 0b11110000;			//Define como saídas: RC0, RC1, RC2 e RC3
    TRISD = 0b00000000;			//Define todos os pinos de PORTD como saídas digitais
    TRISE = 0b00000100;			//Define o bit/pino RE2 como entrada
    OPTION_REG = 0b11010100;	//Configura para o Timer0: Clock interno, incremento na borda
    							//de descida, prescaler dessaciado ao WDT, e com taxa de 1:32.
    ADCON1 = 0b00000100;  		//Define RE0, RE1, RE2 e RA5 como I/O
    T1CON =  0b00000101;		//Habilita o Timer1, clock interno como fonte e prescale 1:1			
   

}	

/*******************************************************************************
* inic_disp
********************************************************************************
* Descrição:		Inicializa o display, configurando-o conforme desejado.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	25/05/2017
* Entrada:		Nenhuma
* Saída:		Nenhuma
*******************************************************************************/
void inic_disp(){
	
	__delay_ms(50);	    	         //Aguarda 50 ms antes de inicializar as configurações.	
	
	PORTEbits.RE0 = 0;				//Seleciona o envio de uma instrução para o display
	PORTD =	0b00111111;	    		//Coloca o display em modo (5x7) e modo de 2 linhas
	PORTEbits.RE1 = 1;			    //Inicia o pulso para efetuar as configurações acima
	__delay_us(1);			        //Aguarda 1 microssegundo (duração do pulso)
	PORTEbits.RE1 = 0;				//Finaliza o pulso, configurando o display conforme descrito acima.			
	
	__delay_ms(1);			        //Aguarda 1 milissegundo para prosseguir com as configurações.
						
	PORTD =	0b00111111;	    		//Coloca o display em modo (5x7) e modo de 2 linhas					|Repete o comando Function Set conforme
	PORTEbits.RE1 = 1;			    //Inicia o pulso para efetuar as configurações acima				|solicitado pelo datasheet.
	__delay_us(1);					//Aguarda 1 microssegundo (duração do pulso)						|
	PORTEbits.RE1 = 0;				//Finaliza o pulso, configurando o display conforme descrito acima.	|
	
	__delay_ms(1);                  //Aguarda 1 milissegundo para prosseguir com as configurações.
	
	PORTD =	0b00111111;	    		//Coloca o display em modo (5x7) e modo de 2 linhas					|Repete o comando Function Set novamente conforme
	PORTEbits.RE1 = 1;			    //Inicia o pulso para efetuar as configurações acima				|solicitado pelo datasheet.
	__delay_us(1);					//Aguarda 1 microssegundo (duração do pulso)						|
	PORTEbits.RE1 = 0;				//Finaliza o pulso, configurando o display conforme descrito acima.	|
	
	__delay_ms(1);      			//Aguarda 1 milissegundo para prosseguir com as configurações.
	
	PORTD = 0b00001100;				//Desliga o cursor, liga o display e desativa o blink do cursor.
	PORTEbits.RE1 = 1;				//Inicializa o pulso para configurar conforme acima 
	__delay_us(1);					//Aguarda 1 microssegundo (duração do pulso)
    PORTEbits.RE1 = 0;				//Finaliza o pulso, configurando o display conforme descrito acima.
	__delay_ms(1);					//Aguarda 1 milissegundo para prosseguir com as configurações.	
	
	PORTD = 0b00000001;				//Limpa o display
	PORTEbits.RE1 = 1;				//Inicia o pulso para efetuar as configurações acima
	__delay_us(1);					//Aguarda 1 microssegundo (duração do pulso)
	PORTEbits.RE1 = 0;				//Finaliza o pulso, configurando o display conforme descrito acima.
	
	__delay_ms(1);					//Aguarda 1 milissegundo para prosseguir com a configuração inicial do display.

	PORTD = 0b00000110;				//Define deslocamento do cursor para à direita, e não permite que a mensagem se desloque a cada nova escrita.
	PORTEbits.RE1 = 1;				//Inicia o pulso para efetuar as configurações acima
	__delay_us(1);					//Aguarda 1 microssegundo (duração do pulso)
	PORTEbits.RE1 = 0;				//Finaliza o pulso, configurando o display conforme descrito acima.
    
    __delay_ms(2);					//Aguarda 2 ms para finalizar a inicialização do display 
}


/*******************************************************************************
* Estado1
********************************************************************************
* Descrição:	Função para varredura do teclado.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	25/05/2017
* Entrada:		Nenhuma
* Saídas:		Valor pressionado no teclado, e o mesmo convertido para inteiro.
*******************************************************************************/
void Estado1(){
    	
	if(INTCONbits.T0IF == 1){    	                 //Se houve o estouro do timer 0, ou seja, se passaram 2,048 ms, então efetua a leitura de uma das linhas do teclado,	
    								                 //conforme sequência, como algoritmo de debouncing. Ao final da leitura, limpa-se a flag, e sai do estado atual.
		if(Tvarredura == 0){						 //Se a linha do teclado a ser varrida for a linha 1:
    
    		PORTC = 0b11111110;				         //Ativa a leitura da primeira linha do teclado
			Ul1 = PORTB;				    		 //Armazena em Ul1 o valor da última leitura da linha 1
			Ul1 = PORTB;				    		 //Armazena em Ul1 o valor da última leitura da linha 1
			Ul1 = PORTB;				    		 //Armazena em Ul1 o valor da última leitura da linha 1
			Ul1 = PORTB;				    		 //Armazena em Ul1 o valor da última leitura da linha 1									
			Atpl1 = ~Atpl1;		            		 //Faz o complementar da antepenúltima leitura da linha1.
			Status1 = Atpl1 | Pl1 | Ul1;			 //Efetua o debouncing
   			Atpl1 = Pl1;							 //Antepenúltima leitura agora é a última leitura
   			Pl1 = Ul1;								 //Penúltima leitura agora é a a última leitura.
       
	       	 if((Status1 & 0b00000001) == 0){		 //Se a tecla '0' for pressionada:
    				valor = '0';					 //Valor recebe '0' para ser mostrado no display posteriormente	
    				num = (int)valor - 0x30;		 //Num recebe o inteiro 0.	
    				EstadoAtual = 3;				 //Próximo estado a ser executado será o terceiro.	
    				if(OpCompleta == 1){			 //Se Concluída uma operação:(Variável "setada" no Terceiro Estado)	
    					OpCompleta = 0;				 //Limpa a variável que indica que uma operação foi concluída.	
    					FLimpDisp = 1;				 //Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.	
    				}				
             }
        	 else if ((Status1 & 0b00000010) == 0){	  //Se a tecla '1' for pressionada:	
        			valor = '1';					  //Valor recebe '1' para ser mostrado no display posteriormente		
        			num = (int)valor - 0x30;          //Num recebe o inteiro '0'
        			EstadoAtual = 3;				  //Próximo estado a ser executado será o terceiro.	
        	     	if(OpCompleta == 1){			  //Se Concluída uma operação:(Variável "setada" no Terceiro Estado)	
    					OpCompleta = 0;				  //Limpa a variável que indica que uma operação foi concluída	
    					FLimpDisp = 1;                //Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
             }
             else if((Status1 & 0b00000100) == 0){	  //Se a tecla '2' for pressionada:	
        			valor = '2';					  //Valor recebe '2' para ser mostrado no display posteriormente	
        			num = (int)valor - 0x30;		  //Num recebe o inteiro '2'	
        			EstadoAtual = 3;				  //Próximo estado a ser executado será o terceiro.	 	
        			if(OpCompleta == 1){			  //Se Concluída uma operação:(Variável "setada" no Terceiro Estado)	
    					OpCompleta = 0;			      //Limpa a variável que indica que uma operação foi concluída	
    					FLimpDisp = 1;				  //Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.	
    				}
             } 
             else if((Status1 & 0b00001000) == 0 ){	  //Se a tecla '3' for pressionada:		
        			valor = '3';					  //Valor recebe '3' para ser mostrado no display posteriormente	
        			num = (int)valor - 0x30;		  //Num recebe o inteiro '3'	
        			EstadoAtual = 3;				  //Próximo estado a ser executado será o terceiro	
        			if(OpCompleta == 1){			  //Se Concluída uma operação:(Variável "setada" no Terceiro Estado)	
    					OpCompleta = 0;				  //Limpa a variável que indica que uma operação foi concluída	
    					FLimpDisp = 1;                //Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
             }	
        	else									  	
        	EstadoAtual = 2; 						 //Se nenhuma tecla da linha 1 for pressionada, próximo estado a ser executado é o 2	
        }   
   
        else if(Tvarredura == 1){					//Se, na sequência, a linha do teclado a ser varrida for a linha 2:
    		
    		PORTC = 0b11111101;					    //Ativa a leitura da segunda linha do teclado	
   			Ul2 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1
			Ul2 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1
			Ul2 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1
			Ul2 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1	
   			Ul2 = PORTB;				    		//Armazena em Ul2 o valor da última leitura da linha 2
			Atpl2 = ~Atpl2;		   			 		//Faz o complementar da antepenúltima leitura da linha 2.
	        Status2 = Atpl2 | Pl2 | Ul2;			//Efetua o debouncing	
   		    Atpl2 = Pl2;							//Antepenúltima leitura agora é a última leitura
            Pl2 = Ul2;								//Penúltima leitura agora é a última leitura.
     		
	     	if((Status2 & 0b00000001) == 0){		//Se a tecla '4' for pressionada:	
    			valor = '4';					    //Valor recebe '4' para ser mostrado no display posteriormente
    			num = (int)valor - 0x30;            //Num recebe o inteiro '4'
    			EstadoAtual = 3;                    //Próximo estado a ser executado será o terceiro
    			if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)	
    				OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    				FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    		    }
            }
        	else if((Status2 & 0b00000010) == 0){	//Se a tecla '5' for pressionada:
        		valor = '5';						//Valor recebe '5' para ser mostrado no display posteriormente	
        		num = (int)valor - 0x30;			//Num recebe o inteiro '5'
        		EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        		if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    				OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    				FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    			}
        	}
        	else if((Status2 & 0b00000100) == 0){	//Se a tecla '6' for pressionada:
        		valor = '6';						//Valor recebe '6' para ser mostrado no display posteriormente
        		num = (int)valor - 0x30;			//Num recebe o inteiro '6'
        		EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        		if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    				OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    				FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    			}
        	} 
        	else if((Status2 & 0b00001000) == 0){	//Se a tecla '7' for pressionada:
        		valor = '7';						//Valor recebe '7' para ser mostrado no display posteriormente
        		num = (int)valor - 0x30;			//Num recebe o inteiro '7'
        		EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
            	if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    				OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    				FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    			}
        	}
        	else
        		EstadoAtual = 2;					//Se nenhuma tecla da linha 2 for pressionada, próximo estado a ser executado é o 2
    
        }
    	else if(Tvarredura == 2){
    			
    			PORTC = 0b11111011;					//Ativa a leitura da terceira linha do teclado 
    			Ul3 = PORTB;				        //Armazena em Ul1 o valor da última leitura da linha 1
				Ul3 = PORTB;				        //Armazena em Ul1 o valor da última leitura da linha 1
				Ul3 = PORTB;				        //Armazena em Ul1 o valor da última leitura da linha 1
				Ul3 = PORTB;				        //Armazena em Ul1 o valor da última leitura da linha 1	
    			Ul3 = PORTB;				    	//Armazena em Ul3 o valor da última leitura da linha 3
				Atpl3 = ~Atpl3 ;		    		//Faz o complementar da antepenúltima leitura da linha 3.
				Status3 = Atpl3 | Pl3 | Ul3;		//Efetua o debouncing	
    			Atpl3 = Pl3;						//Antepenúltima leitura agora é a última leitura
    			Pl3 = Ul3;							//Penúltima leitura agora é a a última leitura.
   			   
	   		    if((Status3 & 0b00000001) == 0){	//Se a tecla '8' for pressionada:
    				valor = '8';					//Valor recebe '8' para ser mostrado no display posteriormente
    				num = (int)valor - 0x30;		//Num recebe o inteiro '8'
    				EstadoAtual = 3;				//Próximo estado a ser executado será o terceiro
    				if(OpCompleta == 1){			//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;				//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;				//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
         		}
        		else if((Status3 & 0b00000010) == 0){	//Se a tecla '9' for pressionada:
        			valor = '9';						//Valor recebe '9' para ser mostrado no display posteriormente
        			num = (int)valor - 0x30;			//Num recebe o inteiro '9'
        			EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        			if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
                }
        		else if((Status3 & 0b00000100) == 0){	//Se a tecla 'A' for pressionada:
        			valor = '+';						//Valor recebe '+' para ser mostrado no display posteriormente	
        			EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        			if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
                } 
        		else if((Status3 & 0b00001000) == 0){	//Se a tecla 'B' for pressionada:
        			valor = '-';						//Valor recebe '-' para ser mostrado no display posteriormente	
        			EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        	    	if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
        		}
        		else 
           			EstadoAtual = 2;					//Se nenhuma tecla da linha 3 for pressionada, próximo estado a ser executado é o 2
        }
    	
	    else if(Tvarredura == 3){
    		 	 
    		 	PORTC = 0b11110111; 					//Ativa a leitura da quarta linha do teclado.
   			 	Ul4 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1
				Ul4 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1
				Ul4 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1
				Ul4 = PORTB;				    		//Armazena em Ul1 o valor da última leitura da linha 1	
   				Ul4 = PORTB;				   		    //Armazena em Ul4 o valor da última leitura da linha 4
				Atpl4 = ~Atpl4 ;		                //Faz o complementar da antepenúltima leitura da linha 4.
				Status4 = Atpl4 | Pl4 | Ul4;	        //Efetua o debouncing	
    			Atpl4 = Pl4;						    //Antepenúltima leitura agora é a última leitura
   				Pl4 = Ul4;						     	//Penúltima leitura agora é a a última leitura.
     			
	     		if((Status4 & 0b00000001) == 0){		//Se a tecla 'C' for pressionada:
    				valor = 'X';						//Valor recebe 'X' para ser mostrado no display posteriormente	
    				EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
    				if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;                  //Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
         		}
        		else if((Status4 & 0b00000010) == 0){	//Se a tecla 'D' for pressionada:
        			valor = '/';						//Valor recebe '/' para ser mostrado no display posteriormente	
        			EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        			if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
                }
        		else if((Status4 & 0b00000100) == 0){	//Se a tecla 'E' for pressionada:
        			valor = '=';						//Valor recebe '=' para ser mostrado no display posteriormente
        			EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        			if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
        		} 
        		else if((Status4 & 0b00001000) == 0){`	//Se a tecla 'F' for pressionada:
        			valor = ' ';						//Valor recebe ' ' para ser mostrado no display posteriormente
        			EstadoAtual = 3;					//Próximo estado a ser executado será o terceiro
        			if(OpCompleta == 1){				//Se Concluída uma operação:(Variável "setada" no Terceiro Estado)
    					OpCompleta = 0;					//Limpa a variável que indica que uma operação foi concluída
    					FLimpDisp = 1;					//Indica que o display deve ser limpo antes que haja a escrita do caracter digitado.
    				}
        		}
        		else
        			EstadoAtual = 2;					//Se nenhuma tecla da linha 4 for pressionada, próximo estado a ser executado é o 2
        } 
        
        Tvarredura++;									//Faz com que, a próxima linha do teclado seja varrida, após a passagem de 2,048 ms.
    	INTCONbits.T0IF = 0;
    
	    if(Tvarredura > 3 ){							//Se as 4 linhas já foram varridas:
    		Tvarredura = 0;								//Reinicia varredura.
    	}
   		
   }
  
   else {												//Não passou 2,048 ms:
   	 EstadoAtual = 2;									//Vai para a rotina que mostra no display o conteúdo digitado, ou o resultado da operação,
   }													//de acordo com as últimas teclas pressionadas.
  
}


/*******************************************************************************
* Estado2
********************************************************************************
* Descrição:    Estado onde ocorrerá a escrita no display LCD, confome operação.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	25/05/2017
* Entrada:		Nenhuma
* Saída:		Nenhuma
*******************************************************************************/
void Estado2(){
			
	
	if(FLimpDisp == 1 && PIR1bits.TMR1IF == 1){		//Se já passou o tempo demandado pela última operação, e a variável para limpar o display foi "setada":
		
		PIR1bits.TMR1IF = 0;			//Limpa a flag do timer 1
		FLimpDisp = 0;					//Limpa a variável que indica quando tem que limpar o display
		
		PORTEbits.RE0 = 0;				//Seleciona o envio de uma instrução para o display
		PORTD = 0b00000001;				//Limpa o display
	    PORTEbits.RE1 = 1;				//Inicia o pulso para efetuar a limpeza do display
	    PerderTempo = 1;				//
	    PerderTempo = 1;				//
	    PerderTempo = 1;			    //
	    PerderTempo = 1;			    //	
	    PORTEbits.RE1 = 0;		        //Finaliza o pulso, configurando o display conforme descrito acima.
	    
	    		
	    TMR1L = 0;						//Limpa o registrador TMR1L para escrever no mesmo. Evitar que mude o valor ao escrever em TMR1H
	    TMR1H = 0b11100000;				//Recarrega os registradores para um valor de 57536. Que até 65535 = 8000 * 250 * 10 ^-9 = 2ms.
	    TMR1L = 0b11000000;				//Ou seja, faz com que, aguarde 2 ms antes de efetuar outra operação no display. Tempo requisitado de acordo com o fabricante.
	    goto  FimDisp; 					//Pula para o fim da rotina.
	
	}

	if(valor != 255 && PIR1bits.TMR1IF == 1){		//Se, na variável "valor" há um dígito válido e passou o tempo necessário:
		if(back == 1){								//Se a variável que indica quando deve-se apagar o último valor digitado estiver "setada":
		    
		    PIR1bits.TMR1IF = 0;		//Limpa a flag de estouro do Timer1 para reiniciar a contagem.			
		    back = 0;					//Limpa a variável que indica quando é para apagar o último valor digitado.
		    PORTEbits.RE0 = 0;			//Seleciona o envio de uma instrução para o display			
			PORTEbits.RE1 = 1;			//Inicia o pulso para efetuar a escrita.		
			PORTD = 0b00000100;			//Coloca em modo: Cursor se deslocando para a esquerda.
			PerderTempo = 0;			//PerdeTempo
			PerderTempo = 1;			//PerdeTempo	
			PerderTempo = 2;			//PerdeTempo
			PerderTempo = 3;            //PerdeTempo
			PORTEbits.RE1 = 0;			//Finaliza o pulso, efetuando a escrita 
			back1 = 1;					//"Seta" a variável que indica que deve-se escrever " ", ou seja, apagando assim o último valor digitado.
			
			TMR1L = 0;
	    	TMR1H = 0b11111100;			//Recarrega os registradores do Timer1 com 64736, que até 65535 = 800 * 250 * 10 ^-9 = 200us.
	   	    TMR1L = 0b11100000;			//Aguardando aproximadamente 200 us para efetuar o comando acima.
	    	goto  FimDisp;  			//Salta incondicionalmente para o fim da rotina que exibe os valores no display 
	    	
		}	
		if(back1 == 1){					//Se a variável que indica quando deve-se escrever " " no display estiver com o valor 1:
		    PIR1bits.TMR1IF = 0;		//Limpa a flag do Timer1 para reiniciar a contagem
		    back1 = 0;					//Limpa a variável que indica quando deve-se escrever " " no display
		    
		    PORTEbits.RE0 = 1;			//Seleciona o envio de dados para o display			
			PORTEbits.RE1 = 1;			//Inicia o pulso para efetuar a escrita.		
			PORTD = valor;				//Escreve " " no display, apagando o último valor digitado.
			PerderTempo = 0;			//PerdeTempo
			PerderTempo = 1;			//PerdeTempo	
			PerderTempo = 2;			//PerdeTempo
			PerderTempo = 3;            //PerdeTempo
			PORTEbits.RE1 = 0;			//Finaliza o pulso, efetuando a escrita 
		 
			back2 = 1;					//Seta a variável, para que, entre na próxima estrutura condicional, recolocando o cursor em modo de deslocamento para a direita
			TMR1L = 0;					//Limpa o registrador TMR1L, seguindo a sequência para escrever no TMR1
	    	TMR1H = 0b11111100;			//Aguarda 200 us para efetuar o comando acima no display.
	   	    TMR1L = 0b11100000;			//64736 até 65535 = 800 * 250 * 10 ^-9 = 200us.
	    	goto  FimDisp;				//Salto incondicional para o final da rotina que escreve no display.
	 	
	 	}  
	 	if(back2 == 1){ 				//Se já foi apagado o último caracter mostrado pelo display:	
 			
 			PIR1bits.TMR1IF = 0;		//Limpa a flag de estouro do TMR1 para reiniciar a contagem.
 			back2 = 0;					//Limpa a variável que indica quando deve se colocar o cursor em modo de deslocamento para a direita
   	    	PORTEbits.RE0 = 0;			//Seleciona o envio de uma instrução para o display			
			PORTEbits.RE1 = 1;			//Inicia o pulso para efetuar a escrita.		
			PORTD = 0b00000110;			//Coloca em modo: Cursor se deslocando para a direita.
			PerderTempo = 0;			//PerdeTempo
			PerderTempo = 1;			//PerdeTempo	
			PerderTempo = 2;			//PerdeTempo
			PerderTempo = 3;            //PerdeTempo
			PORTEbits.RE1 = 0;			//Finaliza o pulso, efetuando a escrita 	
		   
		    valor = 255;				//Atribui um valor inválido para a variável "valor", evitando que o display mostre novamente o último caracter digitado.
			TMR1L = 0;					//Limpa o registrador TMR1L, seguindo a sequência para escrever no TMR1
	    	TMR1H = 0b11111100;			//Aguarda 200 us para efetuar o comando acima..
	   	    TMR1L = 0b11100000;			//64736 até 65535 = 800 * 250 * 10 ^-9 = 200us
	    	goto  FimDisp;				//Salta incondicionalmente para o fim da rotina que escreve no display.
			
		}
		else							//Se não é para deletar o último caracter escrito no display:
		    PIR1bits.TMR1IF = 0;		//Limpa a flag do Timer1
			
			PORTEbits.RE0 = 1;			//Seleciona o envio de dados para o display			
			PORTEbits.RE1 = 1;			//Inicia o pulso para efetuar a escrita.		
			PORTD = valor;				//Escreve então, o caracter pressionado no teclado.
			PerderTempo = 0;			//PerdeTempo
			PerderTempo = 1;			//PerdeTempo	
			PerderTempo = 2;			//PerdeTempo
			PerderTempo = 3;            //PerdeTempo
			PORTEbits.RE1 = 0;			//Finaliza o pulso, efetuando a escrita   
		
			valor = 255; 				//Recarrega a variável "valor" com um valor inválido para evitar que display fique mostrando novamente o mesmo caracter.
			   	    	
   	    	TMR1L = 0;					//Limpa o registrador TMR1L, seguindo a sequência para escrever no TMR1
	    	TMR1H = 0b11111100;			//Aguarda 200 us para efetuar a operação acima.
	   		TMR1L = 0b11100000;			//64736 até 65535 = 800 * 250 * 10 ^-9 = 200us.
	    	goto  FimDisp;				//Salta incondicionalmente para o fim da rotina que escreve no display.
   	 }
   	 
   	 if(linha2 == 1 && PIR1bits.TMR1IF == 1 ){	//Se a operação desejada foi realizada com sucesso, e passou o tempo necessário:
		
		if(DeslocaCursor == 1){				//Se a variável que indica quando o cursor deve se deslocar está "setada":
			
			PIR1bits.TMR1IF = 0;			//Limpa a flag de estouro do timer1
			DeslocaCursor = 0;				//Limpa a variável de indicação que o cursor deve se deslocar.
			
			PORTEbits.RE0 = 0;				//Seleciona o envio de uma instrução para o display	
			PORTEbits.RE1 = 1;				//Inicia o pulso para efetuar a escrita.
	    	PORTD = 0xC0;					//Desloca o cursor para a segunda linha do display
			PerderTempo = 0;				//PerdeTempo
    		PerderTempo = 0;				//PerdeTempo	
			PerderTempo = 0;				//PerdeTempo
			PerderTempo = 0;				//PerdeTempo
			PORTEbits.RE1 = 0;			 	//Finaliza o pulso, efetuando a instrução.
		
			TMR1L = 0;						//Limpa o registrador TMR1L, seguindo a sequência para escrever no TMR1		
	    	TMR1H = 0b11110000;				//Carrega nos registradores o valor de: 61536, 
	   		TMR1L = 0b01100000;				//que até 65535 = 4000 * 250 * 10 ^-9 = 1 ms.
	    	goto  FimDisp;					//Salta incondicionalmente para o fim da rotina de display.
		
		}
		
		else							//Se o cursor já estiver posicionado na linha de baixo:
		{	
			PIR1bits.TMR1IF = 0;		//Limpa o flag de estouro do Timer1
   			PORTEbits.RE0 = 1;			//Seleciona o envio de dados para o display	
			PORTEbits.RE1 = 1;			//Inicia o pulso para efetuar a escrita.		
			PORTD = res[i];			    //Mostra o caracter na posição "i" da string que contém o resultado.
			PerderTempo = 0;			//Perde Tempo
			PerderTempo = 0;			//Perde Tempo	
			PerderTempo = 0;			//Perde Tempo
			PORTEbits.RE1 = 0;			//Finaliza o pulso, efetuando a escrita.
			i++;						//Incrementa 1 para mostrar, na próxima execução da estrutura condicional, o valor na posição seguinte da string
		
			if (i < cont)				//Se ainda não mostrou todo o conteúdo da String:
			{		
		    	TMR1L = 0;				//Limpa o registrador TMR1L, seguindo a sequência para escrever no TMR1	
	    		TMR1H = 0b11110000;		//Carrega nos registradores do Timer1: 61680, que 
	   			TMR1L = 0b11110000;	    //até 65535 = 3855 * 250 * 10 ^-9 = 950 ms.
	    		goto  FimDisp;			//Salto incondicional para o fim da rotina que mostra no display
	    	}   	
			else						//Se já mostrou todo o resultado:	
			{	
				i = 0;					//Zera a contagem das posições da String
				linha2 = 0;		 		//Zera a variável que indica uma operação completa
        		OpCompleta = 1;			//Confirma a execução que mostra o resultado para que, na próxima tecla pressionada, limpe o display antes de mostrar na mesma.
        		TMR1L = 0;				//Limpa o registrador TMR1L, seguindo a sequência para escrever no TMR1	
	    		TMR1H = 0b11110000;		//Carrega nos registradores do Timer1: 61680, que 
	   			TMR1L = 0b11110000;		//até 65535 = 3855 * 250 * 10 ^-9 = 950 ms.
   			}
  		} 			     		
    }
	 FimDisp:							//Label para finalizar a execução da rotina do display.
	 EstadoAtual = 1;					//Volta para o Estado 1.	 
}


/*******************************************************************************
* Estado3
********************************************************************************
* Descrição:    Estado onde ocorrerá o "processamento", quando houver uma tecla pressionada,
				e a operação realizada com os operandos de acordo com o desejado.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	25/05/2017
* Entrada:		Valor correspondente a tecla pressionada e seu número convertido
* Saída:		Resultado da operação solicitada.
*******************************************************************************/
void Estado3(){

		if(EstadoSecundario == 1){		//Se o subestado for o primeiro (armazenar o sinal do primeiro operando):
			Estado31();					//Chama a rotina responsável pela tarefa citada na linha anterior.
	    }
	    else if(EstadoSecundario == 2){	//Se o subestado for o segundo (armazenar o primeiro operando):
	    	Estado32();					//Chama a rotina responsável pela tarefa citada na linha anterior
	    }
	    else if(EstadoSecundario == 3){	//Se o subestado for o terceiro(armazenar o sinal do segundo operando):
	    	Estado33();					//Chama a rotina responsável pela tarefa citada na linha anterior
	    }
	    else if(EstadoSecundario == 4){	//Se o subestado for o quarto(obter o resultado da operação solicitada):
	    	Estado34();					//Chama a rotina responsável pela tarefa citada na linha anterior
	    }
   EstadoAtual = 1;						//Volta para efetuar a varredura do teclado.
}


/*******************************************************************************
* Estado3.1
********************************************************************************
* Descrição:    Função para armazenar o sinal do primeiro operando. E, manter 
				a sequência se os valores forem digitados corretamente.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	30/05/2017
* Entrada:	    Valor correspondente a tecla pressionada, e seu respectivo número convertido
* Saída:		Primeiro valor correspondente ao primeiro Operando, e seu sinal.
*******************************************************************************/
void Estado31(){
	
	if(valor == '+'){				//Se o sinal do primeiro Operando for positivo:
		EstadoSecundario = 1;		//Subestado atual se mantém o primeiro.
	}
	else if (valor == '-'){			//Se o sinal do primeiro Operando for negativo:
		sinal1 = '-';				//Variável sinal1, que armazena o sinal do primeiro Operando, recebe "-"
		EstadoSecundario = 1;		//Subestado atual se mantém o primeiro.
	}
    else if(valor == ' '){			//Se a tecla digitada corresponde ao " " (backspace):
    	EstadoSecundario = 1;		//Subestado atual se mantém o primeiro. Nada é atualizado no operando.	
    }
    else if(valor == 'X'){			//Se a tecla digitada corresponde à "X" (multiplicação)
    	EstadoSecundario = 1;		//Subestado atual se mantém o primeiro. Nada é atualizado no operando.
    }
    else if(valor == '/'){			//Se a tecla digitada corresponde à "/" (divisão)
    	EstadoSecundario = 1;		//Subestado atual se mantém o primeiro. Nada é atualizado no operando.
    }
    else							//Se a tecla pressionada for uma tecla correspondente a um número:
    	EstadoSecundario = 2;		//Próxima função a ser executada é a função que atualizará o valor do operando 1
    	Operando1 = num;			//Operando1 recebe o valor correspondente (saída).	

}


/*******************************************************************************
* Estado3.2
********************************************************************************
* Descrição:    Função que atualiza e armazena o(s) valor(es) do primeiro Operando.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	30/05/2017
* Entrada:		Tecla pressionada e seu valor convertido de acordo com o número correspondente.
* Saída:		O operando 1 e o respectivo sinal da operação desejada.
*******************************************************************************/
void Estado32(){
	
	if(valor != '+' && valor != '-' && valor != '=' && valor != '/' && valor != 'X' && valor != ' ' ){	//Se o valor digitado corresponde à um número:
	
        if(UDC == 0){								//Se for o segundo dígito do operando:
			Operando1 = (Operando1 * 10) +  num;	//Atualiza o conteúdo do operando multiplicando por 10 e somando ao valor recebido
			UDC++;									//Incrementa 1 ao conteúdo da variável que controlará o número máximo de caracteres que o operando pode ter.
		}
		else if(UDC == 1){							//Se for o terceiro dígito do operando:
			Operando1 = (Operando1 * 10) + num;		//Atualiza o conteúdo do operando multiplicando por 10 e somando ao valor recebido
			UDC++;									//Incrementa 1 ao conteúdo da variável que controlará o número máximo de caracteres que o operando pode ter.
		}
		else if(UDC == 2){							//Se for o quarto dígito do operando:
			Operando1 = (Operando1 * 10) +  num;	//Atualiza o conteúdo do operando multiplicando por 10 e somando ao valor recebido
			UDC++;									//Incrementa 1 ao conteúdo da variável que controlará o número máximo de caracteres que o operando pode ter.
		}
		else if(UDC == 3){							//Se for o quinto dígito do operando:
			Operando1 = (Operando1 * 10) + num;		//Atualiza o conteúdo do operando multiplicando por 10 e somando ao valor recebido
			UDC++;									//Incrementa 1 ao conteúdo da variável que controlará o número máximo de caracteres que o operando pode ter.
		}
    	else if(UDC == 4){							//Se for o sexto dígito do operando:
    		Operando1 = (Operando1 * 10) + num;		//Atualiza o conteúdo do operando multiplicando por 10 e somando ao valor recebido
    		UDC++;									//Incrementa 1 ao conteúdo da variável que controlará o número máximo de caracteres que o operando pode ter.
    	}
    	else if(UDC > 4){							//Se o usuário tenta inserir um sétimo dígito ao operando:
    		Operando1 = 0;							//O conteúdo do Operando1 é zerado			
    		UDC = 0;								//A variável que conta o número de algarismos também é zerada.
    	}	
 	}
  
 
   else if(valor == '+' ) {							//Se a tecla pressionada for "+":
 			EstadoSecundario = 3;					//Próxima rotina a ser executada (na máquina de estados do terceiro estado) será a que receberá o sinal do Operando 2
 			sinalOp = '+';							//Armazena o valor "+" como sendo, o valor da operação que ocorrerá entre os dois operandos
 			if(sinal1 == '-'){						//Se o sinal digitado no início, para o Operando1 foi negativo:
 				Operando1 = Operando1 * (-1);		//Operando1 recebe (-Operando1)	
 				sinal1 = '+';						//O sinal do Operando 1 é atualizado como postivo, para a próxima operação, caso o usuário não defina um sinal para o mesmo
            }
            UDC = 0;								//Variável para contagem dos algarimos é zerada.
 		
   }
   else if(valor == '-'){							//Se a tecla pressionada for "-":
 			EstadoSecundario = 3;					//Próxima rotina a ser executada será a que receberá o sinal do Operando2
 			sinalOp = '-';							//Armazena o conteúdo "-" para realizar um subtração entre os Operandos
 			if(sinal1 == '-'){						//Se o sinal digitado no início, para o Operando1 foi negativo:
 				Operando1 = Operando1 * (-1);		//Operando1 recebe (-Operando1)
 				sinal1 = '+';						//O sinal do Operando 1 é atualizado como postivo, para a próxima operação, caso o usuário não defina um sinal para o mesmo	
 			}
 			UDC = 0;								//Zera a variável para contagem dos algarismos do primeiro Operando.
 		
   }
   else if(valor == '/'){							//Se a tecla pressionada for "/":
 			EstadoSecundario = 3;					//Próxima rotina a ser executada será a que receberá o sinal do Operando2
 			sinalOp = '/';							//Armazena o conteúdo "/" para realizar uma divisão entre os Operandos
 			if(sinal1 == '-'){						//Se o sinal digitado no início, para o Operando1 foi negativo:
 				Operando1 = Operando1 * (-1);		//Operando1 recebe (-Operando1)
 				sinal1 = '+';						//O sinal do Operando 1 é atualizado como postivo, para a próxima operação, caso o usuário não defina um sinal para o mesmo
 		     }
 		    UDC = 0; 								//Zera a variável para contagem dos algarismos do primeiro Operando.
 	
        }
   
   else if(valor == 'X'){							//Se a tecla pressionada for "X":
 			EstadoSecundario = 3;					//Próxima rotina a ser executada será a que receberá o sinal do Operando2
 			sinalOp = 'X';							//Armazena o conteúdo "X" para realizar uma multiplicação entre os Operandos
 			if(sinal1 == '-'){						//Se o sinal digitado no início, para o Operando1 foi negativo:
 		    	Operando1 = Operando1 * (-1);		//Operando1 recebe (-Operando1)
 		    	sinal1 = '+';						//O sinal do Operando 1 é atualizado como postivo, para a próxima operação, caso o usuário não defina um sinal para o mesmo
 			}
 			UDC = 0;								//Zera a variável para contagem dos algarismos do primeiro Operando.
 		
   }
   else if(valor == '='){							//Se a tecla pressionada for "=":
 			Operando1 = 0;							//O conteúdo do Operando1 é zerado.
 			EstadoSecundario = 1;					//Volta para a rotina de armazenamento do sinal do primeiro Operando (tentativa de operação inválida)
 			UDC = 0;								//Zera a contagem dos algarismos do primeiro Operando
   }	
   else if(valor == ' '){							//Se a tecla pressionada for " " (backspace)
 			Operando1 = (Operando1 / 10);			//Operando1 é atualizado, elimando o último algarismo do mesmo
  			UDC = UDC - 1;							//A contagem dos algarismos do primeiro Operando é decrementada em 1
  			back = 1;								//"Seta" a variável, para que, na exibição do display, elimine o último caracter pressionado.
   }


}

/*******************************************************************************
* Estado3.3
********************************************************************************
* Descrição:    Função que armazena o sinal do Segundo Operando.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	30/05/2017
* Entrada:		Valor convertido, de acordo com a tecla pressionada para o segundo Operando.
* Saída:		Primeiro dígito do Operando2, e o sinal do Operando2.
*******************************************************************************/
void Estado33(){
	
	if(valor == '+'){			//Se o sinal do segundo Operando for positivo:
		EstadoSecundario = 3;	//Subestado atual se mantém o terceiro.
	}
	else if (valor == '-'){		//Se o sinal do segundo Operando for negativo:
		sinal2 = '-';		    //Variável sinal2, que armazena o sinal do segundo Operando, recebe "-"
		EstadoSecundario = 3;	//Subestado atual se mantém o terceiro
	}
    else if(valor == ' '){		//Se a tecla pressionada corresponde à " " (backspace)
    	EstadoSecundario = 1;	//Subestado atual volta a ser o primeiro
    	Operando1 = 0;			//Zera o conteúdo do primeiro operando
    }
    else if(valor == 'X'){		//Se a tecla pressionada corresponde à "X" 
    	EstadoSecundario = 1;	//Subestado atual volta a ser o primeiro
    }
    else if(valor == '/'){		//Se a tecla pressionada corresponde à "/"
    	EstadoSecundario = 1;	//Subestado atual volta a ser o primeiro
    }
    else if(valor == '='){		//Se a tecla pressionada corresponde à "="
    	EstadoSecundario = 1;	//Subestado atual volta a ser o primeiro
    	Operando1 = 0;			//Conteúdo do Operando1 é zerado.
    }
    else						//Se o valor da tecla pressionada corresponde à um número:
    	EstadoSecundario = 4;	//Próximo subestado a ser executado é o responsável por atualizar o conteúdo do Operando 2 e realizar a operação de acordo com o desejado.
    	Operando2 = num ;		//Operando2 passa a ter o conteúdo de num, que é o valor convertido 

}


/*******************************************************************************
* Estado3.4
********************************************************************************
* Descrição:    Função para atualizar o valor do Operando 2, e realizar a Operação desejada.
* Autor:		Juliano Rodrigues Dourado
* Data de Criação:	30/05/2017
* Entrada:		O valor convertido, de acordo com a tecla pressionada
* Saída:		O resultado da operação desejada
*******************************************************************************/
void Estado34(){
	
	int i;					//Cria uma variável local i
	
	if(valor != '+' && valor != '-' && valor != '=' && valor != '/' && valor != 'X' && valor != ' ' ){		//Se o valor digitado corresponder à um número:
	
		if(UDC == 0){				//Se a variável de contagem dos algarismos do segundo operando for 0:
			Operando2 = (Operando2 * 10) +  num;	//Operando 2 é atualizado, multiplicando por 10 o seu conteúdo e somando com o valor convertido correspondente a tecla presssionada
			UDC++;									//Variável de contagem de algarismos do segundo operando é incrementada
		}
		else if(UDC == 1){				//Se a variável de contagem dos algarismos do segundo operando for 1:
			Operando2 = (Operando2 * 10) +  num;	//Operando 2 é atualizado, multiplicando por 10 o seu conteúdo e somando com o valor convertido correspondente a tecla presssionada
			UDC++;									//Variável de contagem de algarismos do segundo operando é incrementada
		}
		else if(UDC == 2){					//Se a variável de contagem dos algarismos do segundo operando for 2:
			Operando2 = (Operando2 * 10) + num;		//Operando 2 é atualizado, multiplicando por 10 o seu conteúdo e somando com o valor convertido correspondente a tecla presssionada
			UDC++;									//Variável de contagem de algarismos do segundo operando é incrementada
		}
		else if(UDC == 3){					//Se a variável de contagem dos algarismos do segundo operando for 3:
			Operando2 = (Operando2 * 10) + num;		//Operando 2 é atualizado, multiplicando por 10 o seu conteúdo e somando com o valor convertido correspondente a tecla presssionada
			UDC++;									//Variável de contagem de algarismos do segundo operando é incrementada
		}
    	else if(UDC == 4){					//Se a variável de contagem dos algarismos do segundo operando for 4:
    		Operando2 = (Operando2 * 10) +  num;    //Operando 2 é atualizado, multiplicando por 10 o seu conteúdo e somando com o valor convertido correspondente a tecla presssionada
    		UDC++;									//Variável de contagem de algarismos do segundo operando é incrementada
    	}
    	else if(UDC > 4){					//Se a variável de contagem dos algarismos do segundo operando for maior que 5  (ou seja, houve a tentativa de inserir um sétimo algarismo):
    		Operando2 = 0;					//Operando 2 é zerado
    		UDC = 0;						//Contagem dos algarismos do segundo Operando também é zerado
    	}
    }
  
 
    else if(valor == '+' ) {				//Se não, se a tecla pressionada for correspondente ao valor "+":
 			EstadoSecundario = 1;			//Subestado atual volta a ser o primeiro subestado
 			Operando2 = 0;					//O conteúdo do segundo operando é zerado
 			Operando1 = 0;					//O conteúdo do primeiro Operando também é zerado
 			UDC = 0;						//A contagem dos algarismos do segundo operando também é zerada.
    }
 	else if(valor == '-'){					//Se não, se a tecla pressionada for correspondente ao valor "-":
 			EstadoSecundario = 1;			//Subestado atual volta a ser o primeiro subestado
 			Operando2 = 0;					//O conteúdo do segundo operando é zerado
 			Operando1 = 0;					//O conteúdo do primeiro Operando também é zerado
 			UDC = 0;						//A contagem dos algarismos do segundo operando também é zerada.
    }
 	else if(valor == '/'){					//Se não, se a tecla pressionada for correspondente ao valor "/":
 			EstadoSecundario = 1;			//Subestado atual volta a ser o primeiro subestado
 			Operando2 = 0;					//O conteúdo do segundo operando é zerado
 			Operando1 = 0;					//O conteúdo do primeiro Operando também é zerado
 			UDC = 0;						//A contagem dos algarismos do segundo operando também é zerada.
    }
 	else if(valor == 'X'){					//Se não, se a tecla pressionada for correspondente ao valor "X":
 			EstadoSecundario = 1;			//Subestado atual volta a ser o primeiro subestado
 			Operando2 = 0;					//O conteúdo do segundo operando é zerado
 			Operando1 = 0;					//O conteúdo do primeiro Operando também é zerado
 			UDC = 0;						//A contagem dos algarismos do segundo operando também é zerada.
    }
	 else if(valor == '='){					//Se não, se a tecla pressionada for correspondente ao valor "=":
	 
	 		linha2 = 1;						//"Seta" a variável linha2 que confirma que houve uma operação realizada
	 		UDC = 0;						//A contagem dos algarismos do segundo operando também é zerada.
	 		EstadoSecundario = 1;			//Subestado atual volta a ser o primeiro.
	   
	   		 if(sinal2 == '-'){				//Se o sinal do segundo operando é negativo:
 		   		Operando2 = Operando2 * (-1);	//Operando2 = -Operando2
 		   		sinal2 = '+';				//sinal2 é atualizado para positivo para a próxima operação (caso o usuário não digite o sinal do mesmo)
 			 }
 	    	 if(sinalOp == '+'){			//Se a operação escolhida foi uma adição:
 	      		resultado = (float)Operando1 + (float)Operando2;	//resultado recebe a soma dos Operandos
 	         }
 	    	 else if(sinalOp == '-'){		//Se a operação escolhida foi uma subtração:
 	      		resultado = (float)Operando1 - (float)Operando2;	//resultado recebe a subtração dos Operandos
 	    	 }
 	    	 else if(sinalOp == 'X'){		//Se a operação escolhida foi uma multiplicação:
 	      		resultado = (float)Operando1 * (float)Operando2;    //resultado recebe o produto dos Operandos	
 	         }
 	    	 else if(sinalOp == '/'){		//Se a operação escolhida foi uma divisão:
 	      		resultado = (float)Operando1 / (float)Operando2;	//resultado recebe a divisão de Operando 1 por Operando 2	
 	    	 }
 	   
 	 		result = (unsigned long)resultado;		//Salva a parte inteira do resultado
 	 		ltoa(res,result,10);  					//Converte a mesma para uma string (para ser mostrado no display)
 	 		cont = strlen(res);						//Armazena-se o tamanho da string resultante da conversão
 	 		aux1 = (unsigned long)((resultado-result) * 100);	//Armazena-se as duas casas decimais do resultado
 	   
 	 		res[cont] = (',');						//Adiciona-se uma vírgula ao fim da string que contém a parte inteira do resultado
 		    ltoa(aux3,aux1,10);						//Converte as duas casas decimais para string
 	 		res[cont+1] = aux3[0];					//Posição imediatamente posterior à vírgula na string com o resultado inteiro recebe a primeira casa decimal
 	 		res[cont+2] = aux3[1];					//A segunda posição imediatamente posterior à vírgula na string com o resultado inteiro recebe a segunda casa decimal
 	 		res[cont+3] = '\0';						//Define o fim da string após as operação anteriores
 	 		cont = strlen(res); 	 				//Armazena-se o tamanho da String com o resultado completo (ParteInteira,ParteFracionária)	  	 
 	 	   	DeslocaCursor = 1;						//"Seta" a variável DeslocaCursor para definir que o cursor deve-se deslocar para a próxima linha do display para exibir o resultado
 	 			
     
     }
	
     else if(valor == ' '){							//Se não, se o valor correspondente a tecla pressionada for " "(backspace)
 		Operando2 = (Operando2 / 10);				//Elimina o última algarismo digitado para o Operando2
 		back = 1;									//"Seta" a variável back, indicando para a rotina do display que se deve apagar o último caracter exibido no mesmo
 		UDC = UDC - 1;								//Decrementa 1 da variável de contagem dos algarismos do segundo Operando.
 	 }
 
}



