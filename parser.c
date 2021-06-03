#include "anlex.h"

// Variables 
token t;
FILE *archivo;
FILE *output;
char message[41];
char trad[41];
short boolean_error=0;
short tab_level=0;
short tab_band=0;
short cont=0;


//prototipos de funciones para cada NO TERMINAL
void json(void);
void element(void);
void array(void);
void array2(void);
void element_list(void);
void element_list2(void);
void object(void);
void object2(void);
void attribute_list(void);
void attribute_list2(void);
void attribute(void);
void attribute_name(void);
void attribute_value(void);

//implementacion de funciones

char* tabulador(char*);
void destabulador(char*);

void error_message(char* message){
    // Función de mensaje de error.
    boolean_error=1;
    printf("Line %d:\t\nSYNTAX ERROR \n\t%s.\n",line(),message);
}

char* tabulador(char* texto){
    char *tabs= malloc(sizeof(texto));
    for(short i=0;i<tab_level;i++){
        strcat(tabs,"\t");
    }
    strcat(tabs,texto);
    return tabs;
}

void destabulador(char* texto){   
    char *pr = texto, *pw = texto;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != '\t');
    }
    *pw = '\0';
}

void getToken(void) {
    //retorna la siguiente entrada de la fuente
    sigLex();
}

void match(char* c){
    //verifica si el token actual es igual al token a comparar
    //y luego llama al siguiente token de la fuente
    if (strcmp(t.componenteLex, c)==0)
        getToken(); 
    else
        error_message("Match error");
}

void json(){
    //se llama a la regla gramatical element
    element();
    //se llega al final del archivo
    match("EOF");
}

void element(){
    //element: regla de la gramatica

    if(strcmp(t.componenteLex,"L_LLAVE")==0){
        //si el token es una llave abierto, se espera que sea un objeto
        object();
    }else if (strcmp(t.componenteLex,"L_CORCHETE")==0){
        //si el token es corchete abierto, se espera que sea una lista
        array();    
    }else if(strcmp(t.componenteLex,"R_CORCHETE")==0 || strcmp(t.componenteLex,"R_LLAVE")==0||strcmp(t.componenteLex,"COMA")==0){
         //si el token es llave o corchete cerrada, o una coma, significa error
        sprintf(message,"Expected tokens \"{\" o \"[\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        //si no es ninguno de lo anterior, se lee el siguiente token
        getToken();
    tab_level--;
}

void array(){
    //array: regla de la gramatica
    if(strcmp(t.componenteLex,"L_CORCHETE")==0){
        //si el token es una corchete abierto, se hace match y se llama a array prima  
        match("L_CORCHETE"); 
        array2();
        if(tab_band==1)
            fputs("",output);
        tab_band=0;
        
    }else if(strcmp(t.componenteLex,"R_CORCHETE")==0||strcmp(t.componenteLex,"R_LLAVE")==0||strcmp(t.componenteLex,"COMA")==0){
        //si el token actual es una llave o corchete cerrado, significa un error 
        sprintf(message,"Expected tokens \"[\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken();
}

void array2(){
    //si el token es un corchete cerrado
    if(strcmp(t.componenteLex,"R_CORCHETE")==0){
        match("R_CORCHETE");
        tab_band=1;
    }else if(strcmp(t.componenteLex,"L_CORCHETE")==0||strcmp(t.componenteLex,"L_LLAVE")==0){
        //si el token es un corchete abierto
        element_list();
        match("R_CORCHETE");
    }else if(strcmp(t.componenteLex,"R_LLAVE")==0){
        //si el token es una llave cerrada
        sprintf(message,"Expected token \"[\" or \"]\" or \"{\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else
        getToken();
}

void element_list(){
    //si el token es un corchete abierto o una llave abierta
    if(strcmp(t.componenteLex,"L_CORCHETE")==0||strcmp(t.componenteLex,"L_LLAVE")==0){
        element();
        element_list2();   
    }else if(strcmp(t.componenteLex,"R_CORCHETE")==0){
        //si el token es un corchete cerrado
        sprintf(message,"Expected token \"[\" or \"{\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken();    
}

void element_list2(){
    //si el token es una coma
    if(strcmp(t.componenteLex,"COMA")==0){
        match("COMA");
        tab_level++;
        element();
        element_list2();
    }else if(strcmp(t.componenteLex,"R_CORCHETE")!=0){       
        //si el token es un corchete cerrado  
        getToken();
    }
}

void object(){
    //si el token es una llave abierta
    if(strcmp(t.componenteLex,"L_LLAVE")==0){
        fputs("\n",output);
        match("L_LLAVE");
        object2();
        sprintf(trad,"\t</%s>\n","item");
        strcpy(trad,tabulador(trad));
        if(strcmp(t.componenteLex,"EOF")!=0)
            fputs(trad,output);
        if(tab_band==1)
            fputs("\n",output);
        tab_band=0;
    }else if(strcmp(t.componenteLex,"R_CORCHETE")==0||strcmp(t.componenteLex,"R_LLAVE")==0||strcmp(t.componenteLex,"COMA")==0){
        //si el token es un corchete cerrado, una llave cerrada o una coma, significa error
        sprintf(message,"Expected tokens \"{\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken();
}

void object2(){
    //si el token es una llave cerrada
    if(strcmp(t.componenteLex,"R_LLAVE")==0){
        match("R_LLAVE");
    }else if(strcmp(t.componenteLex,"LITERAL_CADENA")==0){
        //si el token es un string
        tab_band=1;
        if(cont!=0){
            sprintf(trad,"<%s>\n","item");
            tab_level++;
            strcpy(trad,tabulador(trad));
            tab_level++;
        }
        cont++;
        attribute_list();
        
        match("R_LLAVE");
    }else if(strcmp(t.componenteLex,"R_CORCHETE")==0||strcmp(t.componenteLex,"COMA")==0){
        //si el token es un corchete cerrado o una coma
        sprintf(message,"Expected tokens \"}\" or \"string\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken();
    tab_level--;  
}

void attribute_list(){
    //si el token es un string
    if(strcmp(t.componenteLex,"LITERAL_CADENA")==0){
        fputs(trad,output);
        attribute();
        attribute_list2();
        tab_level--;
    }else if(strcmp(t.componenteLex,"R_LLAVE")==0){
        //si el token es una llave cerradas
        sprintf(message,"Expected tokens \"string\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken(); 
}

void attribute(){
    //si el token es un string
    if(strcmp(t.componenteLex,"LITERAL_CADENA")==0){
        //puntero auxiliar para extraer las comillas.
        char* aux=t.p->lexema;
        aux++;
        aux[strlen(aux)-1] = '\0';
        sprintf(trad,"<%s>",aux);
        strcpy(trad,tabulador(trad)); // Tabulador devuelve las tabulaciones según el nivel
        fputs(trad,output); // Se escribe en el archivo;
        attribute_name();
        match("DOS_PUNTOS");
        attribute_value();
        destabulador(trad); // Se remueve todas las tabulaciones.
        sprintf(trad,"</%s>\n", aux);
        fputs(trad,output); // Se escribe en el archivo;
    }else if(strcmp(t.componenteLex,"R_LLAVE")==0||strcmp(t.componenteLex,"COMA")==0){
        sprintf(message,"Expected tokens \"string\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken();    
}

void attribute_list2(){
    //si el token es una coma
    if(strcmp(t.componenteLex,"COMA")==0){
        match("COMA");
        attribute();
        attribute_list2();
    }else if(strcmp(t.componenteLex,"R_LLAVE")!=0){
        //si el token es una llave cerrada
        getToken();
    }       
}

void attribute_name(){
    //se espera un string
    if(strcmp(t.componenteLex,"LITERAL_CADENA")==0){
        match("LITERAL_CADENA");
    }else
        getToken();
}

void attribute_value(){
    //se espera una llave abierta
    if(strcmp(t.componenteLex,"L_LLAVE")==0||strcmp(t.componenteLex,"L_CORCHETE")==0){ 
        element();
    }//se espera una cadena
    else if(strcmp(t.componenteLex,"LITERAL_CADENA")==0){
        fputs(t.p->lexema,output);
        match("LITERAL_CADENA");
    }//se espera un numero
    else if(strcmp(t.componenteLex,"LITERAL_NUM")==0){
        fputs(t.p->lexema,output);
        match("LITERAL_NUM");
    }//se espera un TRUE
    else if(strcmp(t.componenteLex,"PR_TRUE")==0){   
        fputs(t.p->lexema,output);     
        match("PR_TRUE");
    }//se espera un FALSE
    else if(strcmp(t.componenteLex,"PR_FALSE")==0){
        fputs(t.p->lexema,output);     
        match("PR_FALSE");
    }//se espera un NULL
    else if(strcmp(t.componenteLex,"PR_NULL")==0){
        fputs(t.p->lexema,output);     
        match("PR_NULL");
    }//caso contrario
    else if(strcmp(t.componenteLex,"R_LLAVE")==0||strcmp(t.componenteLex,"COMA")==0||strcmp(t.componenteLex,"DOS_PUNTOS")==0){
        sprintf(message,"Expected tokens \"{\" or \"[\" or \"string\" or \"number\" or \"true\" or \"false\" o r\"null\" not like that \"%s\"", t.p->lexema);
        error_message(message);
    }else 
        getToken(); 
}

void inicio(void){
    //se llama al simbolo inicial de la gramatica
    json(); 
    if(strcmp(t.componenteLex,"EOF")!=0) error_message("Not like that END OF FILE");
}

void parser(){
    while (strcmp(t.componenteLex,"EOF")!=0){
        getToken();
        inicio();
    }   
}

int main(int argc,char* args[]){
    
    //se inicializa la tabla de simbolos
    initTabla();

    //se carga la tabla de simbolos
    initTablaSimb();
     
    if(argc > 1){

        //se verifica la direccion y nombre del archivo    
    	if ( !( archivo=fopen(args[1],"rt") ) ){
            printf("File not found\n");
            exit(1);
    	}

        //se recorre el archivo
    	while (strcmp(t.componenteLex,"EOF")!=0){
            output = fopen( "output.txt", "w" );
            parser();
            //si no se encontraron errores en la fuente
            if(boolean_error==0)
                printf("Correct font syntax.\n");
            fclose(output);
    	}

        //se cierra el archivo
    	fclose(archivo);

    }else{//no se paso el nombre del archivo fuente
    	printf("No file name\n");
    	exit(1);
    }

    return 0;
}
