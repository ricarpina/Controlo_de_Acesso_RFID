#include <pic18.h>
#include <stdio.h>
#define tam_crc 6 		//tamanho crc
#define tam_dados 46 	//32 dados+ 6 zeros(crc) + 8 endereço

int start_bit,inicio_comunicacao=0;
int var_aux_2=0,controler=0;// variaveis para controlo de start_bit
int vec_binario[50],cont,var_aux,cont2,aux_timer,i,imprimir=0;
int vec_resposta[20];
char buffer;

int start_bit,inicio_comunicacao,cont,aux_timer,cont2=0;
char terminador;
int vec_crc_cod[tam_crc] = {1,0,1,1,0,1}, aux_crc[2*tam_crc]={0},quantos_baixa,aux,cont2,aux2,pos_vec_vec_binario,erro=0; // variaveis do CRC

int vec_rececao[12],index=0,usart_ok=0;// Vetor que vai armazenar a resposta do pc

void gerar_crc(){
	for(i=0;i<tam_dados;i++);
	
	pos_vec_vec_binario=0;

    for(aux=0;vec_binario[aux]==0;aux++)//para começar num bit a "1"
    {
        pos_vec_vec_binario++;

    }
    cont2=0;
	
    while(pos_vec_vec_binario<(tam_dados)) //percorre string a enviar
    {

        if(cont2<tam_crc)//6 posiçoes iniciais
        {
            for(cont2=0;cont2<tam_crc;cont2++,pos_vec_vec_binario++)
            {
                aux_crc[cont2]=((vec_binario[pos_vec_vec_binario])^(vec_crc_cod[cont2])); //xor dos 6 iniciais
            }
        }
        for(aux=0;((aux_crc[aux]==0) && ((pos_vec_vec_binario+aux)<tam_dados));aux++) //quantos baixa
        {
			  if(aux==6){break;}
		}
        for(aux2=0;aux2<tam_crc;aux2++) //arrasta para a esquerda n(quantos baixa!) shirft register
        {
            aux_crc[aux2]=aux_crc[aux2+aux];
        }
        while(aux>0) //adiciona aux bits à direita para o aux_crc do endereço
        {
            aux_crc[tam_crc-aux]=vec_binario[pos_vec_vec_binario];
            aux--;
            pos_vec_vec_binario++;
        }

        for(i=0;i<6;i++);
		
        for(aux=0;aux<tam_crc;aux++) //xor
        {
            aux_crc[aux]=((aux_crc[aux])^(vec_crc_cod[aux]));
        }

    } 

	for(i=0;i<tam_crc;i++){ // Se aux_crc diferente de 0, erro=1, caso contrário erro=0;
		if(aux_crc[i]!=0){
		erro=1;
		}
		
	}
}

void receber_codigo(){
	inicio_comunicacao=1;
	start_bit=0;
	cont=0;
	cont2=0;
	
	while(inicio_comunicacao==1){

		while(start_bit>=10){//não tem igual para atrasar um tempo
			if(aux_timer)
			{
				aux_timer=0;						
					if(cont2%2!=0)
					{
	
						vec_binario[cont]=RB1;
						RB2=1;
						RB2=0;				
						cont++;
					
					}
						if(cont==46)
						{
							start_bit=0;
							inicio_comunicacao=0;
							var_aux_2=0;
							break;
						}
			
			}	
					
		}
			
	}
		
}

void resposta()
{
	start_bit=0;
	inicio_comunicacao=1;
	cont=0;
	cont2=0;
	while(inicio_comunicacao==1)
	{
		RB0=1; 		
		while(start_bit>10)
		{
			if(aux_timer)
			{
				aux_timer=0;
				if(cont2%2==0)
				{
					RB0=vec_rececao[cont];
					cont++;	
				}
				if(cont==13)
				{
						start_bit=0;
						inicio_comunicacao=0;
						break;
				}
			}
		}if(cont==13)break;
	}
}


void interrupt no_name(void)
{
	if((INT1IF)&&(INT1IE)){
		INT1IF=0;
		RA0=!RA0;
		controler=1;
	}
		TXIE=0;

	if(RCIF)
	{
		TXIE=0; //Deve-se garantir que a USART não recebe e transmite ao mesmo tempo
		vec_rececao[index]=RCREG;
		index++;
		RCIF=0;	

		if(index==12){ 
			usart_ok=1;
			index=0;
		}	
    }

	
	if((TMR0IF)&&(TMR0IE)){
		RA3=!RA3;
	    TMR0IF=0;
		cont2++;
		aux_timer=1;
		start_bit++;
		TMR0H=0xD8;  //Configuração para 1mS-->500Hz
		TMR0L=0xF0; 
    }	
}

void putch (unsigned char a)
{
	while (TRMT==0);
	TXREG=a;
}

void main(){
	ADCON1=0b00001111;
	TRISA0=0;
	TRISA1=0;
	TRISA2=0;
	TRISA3=0;
	TRISB0=0;
	TRISB1=1;
	TRISB2=0;
	TRISC3=0;
	TRISA4=0; // Pino para testes
	
	//Configurar a USART para a receção
	SYNC=0;
	CREN=1;
	RCIP=1;	
	RCIF=0;
	RCIE=1;
	PEIE=1;//Habilitar interrupções
	IPEN=1;
	GIE=1;
	
	//Configurar interrupção extena	
	INT1IF=0;
	INT1IP=1;
	INTEDG1=1;
	INT1IE=1;


	// Configurar a USART para transmissão
	TXIE=1;
	TXIP=1;
	TXEN=1; //Habilitar a USART
	BRGH=1; //Gerador de Baud rate 
	SPEN=1;
	SPBRG=255;  //Baud Rate de 9600

	
	// Configurar timer 0
	TMR0ON=1;// ACTIVA TIMER
	T08BIT=0; // NUMERO BITS
	T0CS=0; // ACTIVA CLOCK INTERNO 
	PSA=1; //Sem prescaler
	T0SE=1; 
	TMR0IF=0;
	TMR0IE=1;

	RB0=0;
		
	while(1){	
		if((controler==1)&&(var_aux_2==0)){
			TMR0H=0xD8;  //Configuração para 1mS-->500Hz
			TMR0L=0xF0;
			receber_codigo();
			RB0=0;
			gerar_crc();
			
			if(erro==0){
				for(i=0;i<tam_dados;i++){
					printf("%d",vec_binario[i]);
				}
			}
			terminador='a';
			printf("%c",terminador);
			var_aux_2=1;
			controler=0;
			break;
		}
		if(usart_ok){
			usart_ok=0;
			resposta();
			}
			else{RA1=!RA1;}
			for(i=0;i<12;i++){
				RA2=vec_rececao[i];
				for(d=0;d<500;d++);
			}
		}
	}

