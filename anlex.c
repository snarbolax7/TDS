#include "anlex.h"

int consumir;

char cad[5*TAMLEX]; 
token t; 
FILE *archivo; 
char buff[2*TAMBUFF]; 
char id[TAMLEX]; 
int end=0; 
int numLine=1; 
char space[100];


void error(const char* message){
    printf("Line %d: Lexical error. %s.\n", numLine, message);	
}

void sigLex(){
    int i=0;
    char c=0;
    int accept=0;
    int state=0;
    char msg[41];
    input e;
    
    while((c=fgetc(archivo))!=EOF){
		
	    if (c=='\t' || c==' '){
            strcat(space,&(c));
            continue; 
        }else if(c=='\n'){
            numLine++;
            continue;
	    }else if (isalpha(c)){
            i=0;
            do{
                id[i]=tolower(c);
                i++;
	            c=fgetc(archivo); 
	            if (i>=TAMLEX)
                error("Identifier length exceeds buffer size");
            }while(isalpha(c));
            id[i]='\0';
            if (c!=EOF)
	            ungetc(c,archivo); 
            else
                c=0;
            t.p=search(id); 
            strcpy(t.componenteLex,t.p->componenteLex);
            if (strcmp(t.p->componenteLex,"-1")==0){ 
                strcpy(e.lexema,id);
                strcpy(e.componenteLex,"LITERAL_CADENA");
	            insert(e);
	            t.p=search(id);
                strcpy(t.componenteLex,"LITERAL_CADENA");
            }
            break;
	    }else if (isdigit(c)){
            i=0;
            state=0;
            accept=0;
            id[i]=c;
            while(!accept){
                switch(state){
                    case 0: 
                        c=fgetc(archivo);
                        if (isdigit(c)){
                            id[++i]=c;
                            state=0;
                        }else if(c=='.'){
                            id[++i]=c;
                            state=1;
                        }else if(tolower(c)=='e'){
                            id[++i]=c;
                            state=3;
                        }else{
                            state=6;
                        }
                        break;
                    case 1:
                        c=fgetc(archivo);						
                        if (isdigit(c)){
                            id[++i]=c;
                            state=2;
		                }else{
                            sprintf(msg,"This was not expected '%c'",c);
                            state=-1;
		                }
		                break;
                    case 2:
                        c=fgetc(archivo);
		                if (isdigit(c)){
                            id[++i]=c;
                            state=2;
		                }else if(tolower(c)=='e'){
                            id[++i]=c;
                            state=3;
		                }else
                            state=6;
		                break;
                    case 3:
                        c=fgetc(archivo);
                        if (c=='+' || c=='-'){
                            id[++i]=c;
                            state=4;
		                }else if(isdigit(c)){
                            id[++i]=c;
                            state=5;
		                }else{
                            sprintf(msg,"This was not expected '%c'",c);
                            state=-1;
		                }
		                break;
                    case 4:
		                c=fgetc(archivo);
		                if (isdigit(c)){
                            id[++i]=c;
                            state=5;
		                }else{
                            sprintf(msg,"This was not expected '%c'",c);
                            state=-1;
		                }
		                break;
                    case 5:
		                c=fgetc(archivo);
		                if (isdigit(c)){
                            id[++i]=c;
                            state=5;
		                }else{
                            state=6;
		                }
                        break;
                    case 6:
		                if (c!=EOF)
                            ungetc(c,archivo);
		                else
                            c=0;
		                id[++i]='\0';
		                accept=1;
		                t.p=search(id);
		                if (strcmp(t.p->componenteLex,"-1")==0){ 
                            strcpy(e.lexema,id);
                            strcpy(e.componenteLex,"LITERAL_NUM");
                            insert(e);
                            t.p=search(id);
		                }
                        strcpy(t.componenteLex,"LITERAL_NUM");
		                break;
                    case -1:
		                if (c==EOF)
                            error("The end of file was not expected");
		                else
                            error(msg);
                        exit(1);
                }
            }
            break;
	    }else if (c==':'){
            strcpy(t.componenteLex,"DOS_PUNTOS");
            t.p=search(":");
            break;
	    }else if (c==','){
            strcpy(t.componenteLex,"COMA");
            t.p=search(",");
            break;
	    }else if (c=='['){
            strcpy(t.componenteLex,"L_CORCHETE");
            t.p=search("[");
            break;
	    }else if (c==']'){
            strcpy(t.componenteLex,"R_CORCHETE");
            t.p=search("]");
            break;
	    }else if (c=='{'){
            strcpy(t.componenteLex,"L_LLAVE");
            t.p=search("{");
            break;
	    }else if (c=='}'){
            strcpy(t.componenteLex,"R_LLAVE");
            t.p=search("}");
            break;
	    }else if (c=='\"'){
            i=0;
            id[i]=c;
            i++;
            do{
                c=fgetc(archivo);
		        if (c=='\"'){
                    c=fgetc(archivo);
                    if (c=='\"'){
                        id[i]=c;
                        i++;
                        id[i]=c;
                        i++;
                    }else{
                        id[i]='\"';
                        i++;
                        break;
                    }
		        }else if(c==EOF){
                    error("The end of the file was reached without reaching the end of a literal");
		        }else{
                    id[i]=c;
                    i++;
		        }
            }while(isascii(c));
            id[i]='\0';
            if (c!=EOF)
                ungetc(c,archivo);
            else
		        c=0;
            t.p=search(id);
            strcpy(t.componenteLex,t.p->componenteLex);
            if (strcmp(t.p->componenteLex,"-1")==0){
		        strcpy(e.lexema,id);
                strcpy(e.componenteLex,"LITERAL_CADENA");
                insert(e);
		        t.p=search(id);
                strcpy(t.componenteLex,e.componenteLex);
            }
            break;
	    }else if (c!=EOF){
            sprintf(msg,"%c Not expected",c);
            error(msg);
        }
    }
    if (c==EOF){
        strcpy(t.componenteLex,"EOF");
        sprintf(e.lexema,"eof");
        t.p=&e;
    }    
}

int line(){
    return numLine;
}
