#include <pic18.h>
#include <stdio.h>
#define tam_crc 6 		//tamanho crc
#define tam_dados 46 	//32 dados + 6 zeros(crc) + 8 endereço


unsigned char texto[100];
unsigned int vec[30];

unsigned int buffer,dd=0;
unsigned char i;
int variavel=0,p;
int x,t,z,var,num_div,vec_bin[100],vec_cezar[100];
int start_bit,inicio_comunicacao,cont,aux_timer,cont2=0;
int var_aux_2=0,controler=0;// variáveis para controlo de start_bit
int vec_rececao[20]; // vetor que armazena a resposta da PIC Master
int imprimir=0;

// Variaveis para CRC
int endereco[tam_dados];
int vec_crc_cod[tam_crc] = {1,0,1,1,0,1}, aux_crc[2*tam_crc]={0},quantos_baixa,aux,cont2,aux2,pos_vec_endereco;
int trinco=0,avancar;
void receber_codigo();


void convert_binario(){
     x=0;
	for(i=0;i<4;i++){
		t=0;
		var=0;
		num_div=vec[i];
		do{                       
			vec_bin[x]=(num_div%2); // Guardar no vector vec_bin o código binário 
			num_div=(num_div/2);	// Guarda em num_div o resto da divisão
			x++;
			t++;
		}
		while(num_div>0);
        while(t<8){
                   vec_bin[x]=0;
                   x++;
                   t++;
				   }	
		x=x-1;
		t=t-1;	
		do{
			vec_cezar[x-t]=vec_bin[x-var];
			var++;
			t--;		
			}
		while((t+1)>0);x=x+1;
        t=t+1;
	}
	for(i=0;i<32;i++){
       endereco[8+i]=vec_cezar[i];
    }
}

void putch (unsigned char a)
{
	while (TRMT==0);
	TXREG=a;
}

void interrupt xxx()        
{
	TXIE=0;
	if ((INT1IF)&&(INT1IE)){
		INT1IF=0;
		RB5=!RB5;
		controler=1;
	}
	if(RCIF){
		TXIE=0; //Deve-se garantir que a USART não recebe e transmite ao mesmo tempo
		buffer=RCREG;
		vec[dd]=buffer;
		variavel++;
		dd++;
		RCIF=0;		
    }
	if(TMR0IF){
	    TMR0IF=0;
		aux_timer=1;
		cont2++;
		start_bit++;
		trinco++;
		TMR0H=0xD8;  //Configuração para 2mS-->500Hz
		TMR0L=0xF0; 
    }	
}


void gerar_crc(){
	printf("Numero \r\n");
	for(i=0;i<tam_dados;i++){
		printf("%d",endereco[i]);
	}
	for(i=0;i<tam_dados;i++);
	pos_vec_endereco=0;

    for(aux=0;endereco[aux]==0;aux++)//para começar num bit a "1"
    {
        pos_vec_endereco++;

    }
    cont2=0;
	
    while(pos_vec_endereco<(tam_dados)) //percorre string a enviar
    {

        if(cont2<tam_crc)//6 posiçoes iniciais
        {
            for(cont2=0;cont2<tam_crc;cont2++,pos_vec_endereco++)
            {
                aux_crc[cont2]=((endereco[pos_vec_endereco])^(vec_crc_cod[cont2])); //xor dos 6 iniciais
            }
        }
        for(aux=0;((aux_crc[aux]==0) && ((pos_vec_endereco+aux)<tam_dados));aux++) //quantos baixa
        {
			  if(aux==6){break;}
		}
        for(aux2=0;aux2<tam_crc;aux2++) //arrasta para a esquerda n(quantos baixa!) shirft register
        {
            aux_crc[aux2]=aux_crc[aux2+aux];
        }
        while(aux>0) //adiciona aux bits á direita para o aux_crc do endereço
        {
            aux_crc[tam_crc-aux]=endereco[pos_vec_endereco];
            aux--;
            pos_vec_endereco++;
        }
         for(i=0;i<6;i++);
        for(aux=0;aux<tam_crc;aux++) //xor
        {
            aux_crc[aux]=((aux_crc[aux])^(vec_crc_cod[aux]));
        }
    } 
	printf("\r\nXXXXXXXXXX\r\n");
	for(i=0;i<tam_crc;i++){
		printf("CRC[%d]=%d\r\n",i,aux_crc[i]);
	}
	for(i=40,aux=0;i<(tam_dados);i++,aux++){ // Colocar CRC no vetor endereço
		endereco[i]=aux_crc[aux];
		printf("endereco[%d]=%d**** aux_crc[%d]=%d\r\n",i,endereco[i],i,aux_crc[aux]);
	}
}

void enviar_codigo()
{
	start_bit=0;
	inicio_comunicacao=1;
	cont=0;
	cont2=0;
	while(inicio_comunicacao==1)
	{
		RB0=1; 		
		while(start_bit>=10)
		{
			if(aux_timer)
			{
				aux_timer=0;
				if(cont2%2==0)
				{
					RB0=endereco[cont];
					cont++;
				}
				if(cont==47)
				{
						start_bit=0;
						inicio_comunicacao=0;
						RB0=1;
						break;
				}
			}
		}if(cont==47)break;	
	}
}

void receber_codigo(){
	RB4!=RB4;
	controler=0;
	inicio_comunicacao=1;
	start_bit=0;
	cont=0;
	cont2=0;
	var_aux_2=1;
	while(inicio_comunicacao==1){

		while(start_bit>10){//não tem igual para atrasar um tempo
			if(aux_timer)
			{
				aux_timer=0;
				if(cont2%2==0)
				{	
					vec_rececao[cont]=RB1;
					RA4=1;
					RA4=0;
					cont++;
				}
				if(cont==13)		
				{
						controler=1; // Pronto para receber novamente uma interrupção
						RB7=1;
						RB7=0;
						start_bit=0;
						inicio_comunicacao=0;
						controler=0;
						var_aux_2=0;
						imprimir=1; // Para imprimir o vec_receber

						break;
				}
			}				
		}	
	}	
}



void main(){
	ADCON1=0b00001111; 
	TRISA4=0;
	TRISA0=1;
	TRISA1=1;
	TRISA2=1;
	TRISA3=1;
	TRISC0=1;
	TRISC1=1;
	TRISC2=1;
	TRISC3=1;
	TRISC4=0;
	TRISB7=0;
	TRISB5=0;
	TRISB4=0;
	TRISB6=0;
	TRISB0=0;
	TRISB1=1;
	TRISB7=0;
	TRISC6=0;
	TRISC7=1;
	
	// Configurar a USART para transmissão
	TXIE=1;
	TXIP=1;
	TXEN=1; //Habilitar a USART
	BRGH=1; //Gerador de Baud rate 
	SPEN=1;
	SPBRG=255;  //Baud Rate de 9600
	
	//Configurar a USART para a receção
	SYNC=0;
	CREN=1;
	RCIP=1;	
	RCIF=0;
	RCIE=1;
	PEIE=1;//Habilitar interrupções
	IPEN=1;
	GIE=1;
	
	//Configurar interrupção INT1
	INT1IF=0;
	INT1IP=1;
	INTEDG1=1;
	INT1IE=1;
	GIE=1;
	
	// Configurar timer 0 
	TMR0ON=1;// ACTIVA TIMER
	T08BIT=0; // NUMERO BITS
	T0CS=0; // ACTIVA CLOCK INTERNO 
	PSA=1; // Sem prescaler
	T0SE=1; 
	
	TMR0IF=0;
	TMR0IE=1;
	RB0=0;
	RC4=0;

	// Colocar Endereço no "endereco[]"
	for(i=0;i<tam_dados;i++){
		endereco[i]=0;
	}
	endereco[0]=RA0;
	endereco[1]=RA1;
	endereco[2]=RA2;
	endereco[3]=RA3;
	endereco[4]=RC0;
	endereco[5]=RC1;
	endereco[6]=RC2;
	endereco[7]=RC3;
	
	while(1){
		if ((controler)&&(!RC4)){
			TMR0H=0xFF;
			TMR0L=0xFE;
			receber_codigo();
		}
		if(imprimir==1){
			imprimir=0;
			if((vec_rececao[4]==endereco[0])&&(vec_rececao[5]==endereco[1])&&(vec_rececao[6]==endereco[2])
				&&(vec_rececao[7]==endereco[3])&&(vec_rececao[8]==endereco[4])&&(vec_rececao[9]==endereco[5])
				&&(vec_rececao[10]==endereco[6])&&(vec_rececao[11]==endereco[7])){
				avancar=1;
			}
			else{avancar=0;}
			if(avancar){
				if((vec_rececao[0]==0)&&(vec_rececao[1]==0)&&(vec_rececao[2]==0)&&(vec_rececao[3]==0)){RC4=0;}
				else if((vec_rececao[0]==1)&&(vec_rececao[1]==1)&&(vec_rececao[2]==1)&&(vec_rececao[3]==1)){RC4=1;trinco=0;}
			}
		}
		if(RC4){
			if(trinco==20){trinco=0;RC4=0;}
		}
		if(variavel==4){
			variavel=0;
			for(z=0;z<4;z++){
			}
			if(z==4)dd=0;
				convert_binario();
				gerar_crc();
			for(i=0;i<tam_dados;i++){
				printf("%d",endereco[i]);
			}
			enviar_codigo();
			RB0=0;
			for(i=8;i<tam_dados;i++){ // Fazer reset ao vetor endereço
				endereco[i]=0;
			}
			variavel=0;
			for(z=0;z<32;z++);			
			for(z=0;z<6;z++);
		}
	}
}




