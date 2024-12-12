#include "SystemManager.hpp"
#include <iostream>
#include <string>
#include <vector>
bool iscreate(std::string txt){
    if(txt[0]=='c' && txt[1]=='r' &&txt[2]=='e' && txt[3]=='a' && txt[4]=='t' && txt[5]=='e')
    {
       return true; 
    }
    return false;
}
bool iscat(std::string txt){
    if (txt[0] == 'c' && txt[1] == 'a' && txt[2] == 't') {
        return true;
    }
    return false;
}

std::vector<std::string> data(std::string& txt) {
    std::vector<std::string> vec;
    std::string palabra = "";
    bool dentroDelimitadores = false;
    
    for (char ch : txt) {
        if (ch == '<') {
            // Comienza una nueva palabra
            dentroDelimitadores = true;
            palabra = ""; // Reinicia la palabra por si había residuos
        } else if (ch == '>') {
            // Finaliza la palabra y la agrega al vector si es válida
            if (dentroDelimitadores && !palabra.empty()) {
                vec.push_back(palabra);
            }
            dentroDelimitadores = false;
            palabra = ""; 
        } else if (dentroDelimitadores) {
            palabra += ch;
        }
    }
    
    return vec;
}


bool isrm(std::string txt){
    if(txt[0]=='r' && txt[1]=='m'){
        return true;
    }
    return false;
}

bool iscopyin(std::string txt){
    if(txt[0]=='c' && txt[1]=='o' && txt[2]=='p' && txt[3]=='y' && txt[4]==' ' && txt[5]=='i' && txt[6]=='n'){
        return true;
    }
    return false;
}

bool iscopyout(std::string txt){
    //copy out <Local><Externo>.
    if(txt[0]=='c' && txt[1]=='o' && txt[2]=='p' && txt[3]=='y' && txt[4]==' ' && txt[5]=='o' && txt[6]=='u' && txt[7]=='t'){
        return true;
    }
    return false;
}

bool ishexdump(std::string txt){
    if(txt[0] == 'h' && txt[1] == 'e' && txt[2] == 'x' && txt[3]=='d' && txt[4]=='u' &&txt[5]=='m' && txt[6]=='p'){
        return true;
    }
    return false;
}

bool iswrite(std::string txt){
    if (txt[0] == 'w' && txt[1] == 'r' && txt[2] == 'i' && txt[3]=='t' && txt[4]=='e') {
        return true;
    }
    return false;
}



int main() {
    SystemManager admin;
    if(!admin.OpenDisk()){
        std::size_t i;
        std::cout<<"Ingrese el tamaño en bytes de los bloques.\n";
        std::cin>>i;
        if(admin.CreateNewDisk(i)){
            std::cout<<"Se creo un nuevo disco virtual.\n";
            admin.OpenDisk();
        }

    }else{
        std::cout<<"Simulador iniciado correctamete.\n";
    }
    std::string txt="";
    std::string path="/KennyMenjivar/ProyectoFinal/Disk ~ ";

    while(txt!="exit"){
        std::cout<<path;
        std::getline(std::cin, txt);

        if(txt=="help"){
            std::cout << "\nComandos disponibles:\n";
            std::cout << "  format                      - Formatea el disco virtual.\n";//
            std::cout << "  ls                          - Muestra la lista de archivos.\n";//
            std::cout << "  cat <archivo>               - Muestra el contenido de un archivo.\n";//
            std::cout << "  write <texto><archivo>      - Escribe texto en un archivo.\n";//
            std::cout << "  hexdump <archivo>           - Muestra el contenido en hexadecimal.\n";
            std::cout << "  copy in <simulado><host>    - Copia del sistema host al simulado.\n";
            std::cout << "  copy out <simulado><host>   - Copia del simulado al sistema host.\n";
            std::cout << "  rm <archivo>                - Elimina un archivo.\n";//
            std::cout << "  create <NombreArchivo>      -Crea un nuevo archivo en el archivo.\n";
        }else if(txt=="format"){
            admin.format();
        }else if(txt=="ls"){
            admin.ShowArchivos();
        }else if(iscat(txt)){
            std::vector<std::string> vec = data(txt);
            if(vec.size()>0){
                std::cout<<admin.showInfoOfDat(vec[0])<<"\n";
            }else{
                std::cout<<"No se encontro dicho archivo.\n";
            }
        }else if(iswrite(txt)){
            
            std::vector<std::string> vec = data(txt);
            if(vec.size()>1){ 
                admin.write(vec[0],vec[1]);
            }else{
                std::cout<<"No cumple con los requerimientos.\n";
            }
        }else if(ishexdump(txt)){
            std::vector<std::string> vec = data(txt);
            if(vec.size()>0){
                admin.showInfoHexa(vec[0]);
            }else{
                std::cout<<"Error: No ingreso adecuandamente los datos.\n";
            }
        }else if(iscopyout(txt)){
            std::vector<std::string> vec= data(txt);
            if(vec.size()>1){
                admin.copyOut(vec[0],vec[1]);
            }else{
                std::cout<<"Error: No ingreso adecuandamente los datos.\n";
            }
        }else if(iscopyin(txt)){
            std::vector<std::string> vec= data(txt);
            if(vec.size()>1){
                admin.copyIn(vec[0],vec[1]);
            }else{
                std::cout<<"Error: No ingreso adecuandamente los datos.\n";
            }
        }else if(isrm(txt)){
            std::vector<std::string> vec = data(txt);
            if(vec.size()>0){
                admin.deleteArchivo(vec[0]);
            }else{
                std::cout<<"Error: No ingreso adecuandamente los datos.\n";
            }
        }else if(txt=="exit"){
            break;
        }else if(iscreate(txt)){
            std::vector<std::string> vec = data(txt);
            if(vec.size()>0){
                admin.createnewfile(vec[0]);
            }else{
                std::cout<<"Error: No ingreso adecuandamente los datos.\n";
            }
        }else{
            std::cout<<"No se econtro el comando, ingrese el help.\n";
        }
        //txt=" ";
    }
    
    return 0;
}