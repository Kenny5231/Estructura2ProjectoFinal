#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class SystemManager{

    public:
    std::size_t sizeOfBlock;
        struct Header
        {
            std::size_t tamañoBlock;
            std::size_t cantidadArchivos;
            std::size_t TotalInodos;
        };
        std::fstream filename;
        Header header;
        struct Inodos
            {
            char nombre[64];                 
            std::size_t tamaño;             
            std::size_t bloques_usados[8];   
            bool ocupado;
            //147-10 BYTES
            };

        struct superBloque
            {
            std::size_t block_count;          
            std::size_t block_size;           
            std::size_t max_files = 256;      
            std::size_t used_blocks = 0;       
            std::vector<std::size_t> free_map; 
            std::vector<Inodos> TablaInodos;
            //2104 bytes
            };//
        superBloque superbloque;


    bool OpenDisk();
    bool CreateNewDisk(std::size_t i);   
    void ShowArchivos();
    bool write(std::string txt, std::string NombreArchivo);        
    bool ExisteArchivo(std::string NombreArchivo);
    std::vector<std::string> informacionEnBloques(std::string txt);
    std::string showInfoOfDat(std::string archivo);
    void showInfoHexa(std::string archivo);
    bool format();
    bool deleteArchivo(std::string txt);
    bool copyIn(std::string local, std::string externo);
    bool copyOut(std::string local, std::string externo);
    bool createnewfile(std::string nombreArchivo);
};

