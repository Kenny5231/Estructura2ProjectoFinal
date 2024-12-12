#include "SystemManager.hpp"

bool SystemManager::OpenDisk() {
    filename.open("Disk.io", std::ios::in | std::ios::out | std::ios::binary);
    if (!filename.is_open()) {
        return false;
    }

    // Leer el header
    filename.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Leer el superBloque
    filename.read(reinterpret_cast<char*>(&superbloque.block_count), sizeof(superbloque.block_count));
    filename.read(reinterpret_cast<char*>(&superbloque.block_size), sizeof(superbloque.block_size));
    filename.read(reinterpret_cast<char*>(&superbloque.max_files), sizeof(superbloque.max_files));
    filename.read(reinterpret_cast<char*>(&superbloque.used_blocks), sizeof(superbloque.used_blocks));

    superbloque.free_map.resize(256);
    filename.read(reinterpret_cast<char*>(superbloque.free_map.data()), 256 * sizeof(std::size_t));

    // Leer los inodos
    superbloque.TablaInodos.clear();
    for (std::size_t i = 0; i < header.TotalInodos; ++i) {
        Inodos tmp;
        filename.read(reinterpret_cast<char*>(&tmp), sizeof(Inodos));
        superbloque.TablaInodos.push_back(tmp);
    }

    return true;
}

bool SystemManager::CreateNewDisk(std::size_t i) {
    // Calcular parámetros
    sizeOfBlock=i;
    std::size_t inodosXBloque = sizeOfBlock / sizeof(Inodos);
    std::size_t BloquesTaPTabla = 256 / inodosXBloque;

    // Crear el header
    Header header;
    header.tamañoBlock = sizeOfBlock;
    header.cantidadArchivos = 256;
    header.TotalInodos = BloquesTaPTabla * inodosXBloque;

    // Crear el superBloque
    superBloque super;
    super.block_size = sizeOfBlock;
    super.max_files = 256;
    super.block_count = 256;
    super.used_blocks = 0;
    super.free_map = std::vector<std::size_t>(256, 0); // Inicializar mapa de bloques libres

    // Calcular el tamaño del super bloque
    std::size_t tamañoSuperBloque = sizeof(header) + sizeof(super.block_count) + sizeof(super.block_size) +
                                     sizeof(super.max_files) + sizeof(super.used_blocks) +
                                     super.free_map.size() * sizeof(std::size_t);

    // Convertir tamaño a bloques
    std::size_t bloquesSuperBloque = (tamañoSuperBloque + header.tamañoBlock - 1) / header.tamañoBlock;

    // Marcar los bloques ocupados por el super bloque como usados
    for (std::size_t i = 0; i < bloquesSuperBloque; ++i) {
        super.free_map[i] = 1;
    }

    super.used_blocks += bloquesSuperBloque;

    // Crear los inodos
    std::vector<Inodos> inodos;
    for (std::size_t i = 0; i < header.TotalInodos; ++i) {
        Inodos tmp;
        if (i == 0) { // Inodo del super bloque
            std::strncpy(tmp.nombre, "Super Bloque", sizeof(tmp.nombre) - 1);
            tmp.nombre[sizeof(tmp.nombre) - 1] = '\0';
            tmp.tamaño = tamañoSuperBloque;
            tmp.ocupado = true;
            std::fill(std::begin(tmp.bloques_usados), std::end(tmp.bloques_usados), 0);
            for (std::size_t j = 0; j < bloquesSuperBloque && j < 8; ++j) {
                tmp.bloques_usados[j] = j; // Asignar los bloques ocupados
            }
        } else {
            std::fill(std::begin(tmp.nombre), std::end(tmp.nombre), '\0');
            tmp.tamaño = 0;
            tmp.ocupado = false;
            std::fill(std::begin(tmp.bloques_usados), std::end(tmp.bloques_usados), 0);
        }
        inodos.push_back(tmp);
    }

    // Abrir archivo
    if (filename.is_open()) {
        filename.close(); // Cerrar si ya estaba abierto
    }
    filename.open("Disk.io", std::ios::out | std::ios::binary | std::ios::trunc);
    if (!filename.is_open()) {
        std::cerr << "Error: No se pudo crear el disco virtual.\n";
        return false;
    }

    // Escribir el header
    filename.write(reinterpret_cast<char*>(&header), sizeof(header));

    // Escribir el superBloque (sin la tabla de inodos, que se escribe después)
    filename.write(reinterpret_cast<char*>(&super.block_count), sizeof(super.block_count));
    filename.write(reinterpret_cast<char*>(&super.block_size), sizeof(super.block_size));
    filename.write(reinterpret_cast<char*>(&super.max_files), sizeof(super.max_files));
    filename.write(reinterpret_cast<char*>(&super.used_blocks), sizeof(super.used_blocks));
    filename.write(reinterpret_cast<char*>(super.free_map.data()), super.free_map.size() * sizeof(std::size_t));

    // Escribir los inodos
    for (const auto& inodo : inodos) {
        filename.write(reinterpret_cast<const char*>(&inodo), sizeof(Inodos));
    }

    filename.close();
    return true;
}

void SystemManager::ShowArchivos() {
    if (superbloque.TablaInodos.empty()) {
        std::cerr << "No se encontraron archivos en el disco.\n";
        return;
    }

    std::cout << "Archivos en el disco:\n";
    for (std::size_t i = 0; i < superbloque.TablaInodos.size(); ++i) {
        const auto& inodo = superbloque.TablaInodos[i];
        if (inodo.ocupado) {
            std::cout << "Archivo " << i << ", Nombre: " << inodo.nombre << ", Tamaño: " << inodo.tamaño << " bytes\n";
        }
    }
}

bool SystemManager::write(std::string txt, std::string NombreArchivo) {
    if (!filename.is_open()) {
        std::cerr << "Error: Disco virtual no está abierto.\n";
        return false;
    }
    if(ExisteArchivo(NombreArchivo)){
    // Verificar si el archivo ya existe
    Inodos* inodoExistente = nullptr;
    std::size_t inodoIndex = -1;
    for (std::size_t i = 0; i < superbloque.TablaInodos.size(); ++i) {
        if (superbloque.TablaInodos[i].ocupado && std::string(superbloque.TablaInodos[i].nombre) == NombreArchivo) {
            inodoExistente = &superbloque.TablaInodos[i];
            inodoIndex = i;
            break;
        }
    }

    // Dividir el texto en bloques
    std::vector<std::string> bloques = informacionEnBloques(txt);
    std::vector<std::size_t> bloquesUsados;

    // Encontrar bloques libres suficientes
    for (std::size_t i = 0; i < superbloque.free_map.size(); ++i) {
        if (superbloque.free_map[i] == 0 && bloquesUsados.size() < bloques.size()) {
            bloquesUsados.push_back(i);
        }
    }

    if (bloquesUsados.size() < bloques.size()) {
        std::cerr << "Error: No hay suficientes bloques libres para escribir el archivo.\n";
        return false;
    }

    // Escribir los datos en los bloques libres
    for (std::size_t i = 0; i < bloques.size(); ++i) {
        filename.seekp(header.tamañoBlock * bloquesUsados[i], std::ios::beg);
        filename.write(bloques[i].c_str(), bloques[i].size());
        filename.flush();
    }

    // Actualizar el inodo existente o crear uno nuevo
    if (inodoExistente) {
        // Agregar los nuevos bloques al inodo existente
        for (std::size_t i = 0; i < bloquesUsados.size(); ++i) {
            for (std::size_t& bloque : inodoExistente->bloques_usados) {
                if (bloque == 0) {
                    bloque = bloquesUsados[i];
                    break;
                }
            }
        }
        inodoExistente->tamaño += txt.size();

        // Guardar cambios del inodo en el archivo
        filename.seekp(sizeof(header) + sizeof(superbloque.block_count) + sizeof(superbloque.block_size) +
                       sizeof(superbloque.max_files) + sizeof(superbloque.used_blocks) +
                       superbloque.free_map.size() * sizeof(std::size_t) +
                       inodoIndex * sizeof(Inodos), std::ios::beg);
        filename.write(reinterpret_cast<const char*>(inodoExistente), sizeof(Inodos));
    } else {
        // Crear un nuevo inodo
        for (std::size_t i = 0; i < superbloque.TablaInodos.size(); ++i) {
            Inodos& inodo = superbloque.TablaInodos[i];
            if (!inodo.ocupado) {
                std::strncpy(inodo.nombre, NombreArchivo.c_str(), sizeof(inodo.nombre) - 1);
                inodo.nombre[sizeof(inodo.nombre) - 1] = '\0';
                inodo.tamaño = txt.size();
                inodo.ocupado = true;

                for (std::size_t j = 0; j < bloquesUsados.size(); ++j) {
                    if (j < 8) { // Hasta 8 bloques por inodo
                        inodo.bloques_usados[j] = bloquesUsados[j];
                    }
                }

                // Guardar el nuevo inodo en el archivo
                filename.seekp(sizeof(header) + sizeof(superbloque.block_count) + sizeof(superbloque.block_size) +
                               sizeof(superbloque.max_files) + sizeof(superbloque.used_blocks) +
                               superbloque.free_map.size() * sizeof(std::size_t) +
                               i * sizeof(Inodos), std::ios::beg);
                filename.write(reinterpret_cast<const char*>(&inodo), sizeof(Inodos));
                break;
            }
        }
    }
    

    // Marcar los bloques como usados en el mapa de bloques libres
    for (std::size_t bloque : bloquesUsados) {
        superbloque.free_map[bloque] = 1;
    }

    // Guardar el mapa de bloques actualizado en el archivo
    filename.seekp(sizeof(header) + sizeof(superbloque.block_count) + sizeof(superbloque.block_size) +
                   sizeof(superbloque.max_files) + sizeof(superbloque.used_blocks), std::ios::beg);
    filename.write(reinterpret_cast<const char*>(superbloque.free_map.data()),
                   superbloque.free_map.size() * sizeof(std::size_t));

    superbloque.used_blocks += bloquesUsados.size();
    return true;
    
    }
    return false;
}


bool SystemManager::ExisteArchivo(std::string NombreArchivo)
{
    for(int i=0;i<superbloque.TablaInodos.size();i++){
        if(superbloque.TablaInodos[i].nombre==NombreArchivo){
            return true;
        }
    }
    return false;
}

std::vector<std::string> SystemManager::informacionEnBloques(std::string txt) {
    std::vector<std::string> vec;
    std::size_t blockSize = sizeOfBlock; // Tamaño de cada bloque
    std::size_t totalSize = txt.size(); // Tamaño total del texto
    
    for (std::size_t i = 0; i < totalSize; i += blockSize) {
        // Extraer un substring de tamaño blockSize o lo que reste
        vec.push_back(txt.substr(i, blockSize));
    }
    
    return vec;
}
std::string SystemManager::showInfoOfDat(std::string archivo) {
    std::string txt="";
    if (ExisteArchivo(archivo)) {
        for (std::size_t i = 0; i < superbloque.TablaInodos.size(); ++i) {
            Inodos& inodo = superbloque.TablaInodos[i];

            // Comparar correctamente nombres (usar std::strncmp)
            if (std::strncmp(inodo.nombre, archivo.c_str(), sizeof(inodo.nombre)) == 0) {
                // Leer datos de cada bloque usado por el inodo
                for (int j = 0; j < 8; ++j) {
                    if (inodo.bloques_usados[j] == 0) {
                        // Bloques no usados están marcados como 0
                        continue;
                    }

                    std::size_t offset = inodo.bloques_usados[j] * header.tamañoBlock;

                    // Mover el puntero de lectura al bloque correspondiente
                    filename.seekg(offset, std::ios::beg);

                     // Leer el contenido del bloque
                std::vector<char> buffer(header.tamañoBlock);
                filename.read(buffer.data(), buffer.size());

                // Verificar si se leyó correctamente
                if (filename.gcount() > 0) {
                    txt.append(buffer.data(), filename.gcount());
                } else {
                    std::cerr << "Error al leer el bloque en offset: " << offset << "\n";
                }
                }
                return txt;
            }
        }
        return txt;
    } else {
        return txt;
    }
}

void SystemManager::showInfoHexa(std::string archivo) {
    if(ExisteArchivo(archivo)){
        std::string dat= showInfoOfDat(archivo);
        std::cout<<"Contenido de: "<<archivo<<"\n";
     for(unsigned char ch: dat){
            std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(ch)<<" ";
        }
        std::cout<<"\n";
    }else{
        std::cout<<"No se encontro el archivo.\n";
    }
}
bool SystemManager::format() {
for (int i = 0; i < superbloque.TablaInodos.size(); i++) {
    Inodos& inodo = superbloque.TablaInodos[i];
    std::string nombreArchivo(inodo.nombre, strnlen(inodo.nombre, sizeof(inodo.nombre)));
    if (nombreArchivo != "Super Bloque") {
    // Limpiar nombre
        for (int j = 0; j < 64; j++) {
            inodo.nombre[j] = '\0';
        }

        // Liberar bloques usados
        for (int j = 0; j < 8; j++) {
        if (inodo.bloques_usados[j] != 0) {
            superbloque.free_map[inodo.bloques_usados[j]] = 0;
            inodo.bloques_usados[j] = 0;
            }
        }

        // Marcar inodo como libre
        inodo.ocupado = false;

        // Escribir cambios al archivo
        if (filename.is_open()) {
            filename.seekp(24);
            filename.write(reinterpret_cast<char*>(&superbloque.block_count), sizeof(superbloque.block_count));
            filename.write(reinterpret_cast<char*>(&superbloque.block_size), sizeof(superbloque.block_size));
            filename.write(reinterpret_cast<char*>(&superbloque.max_files), sizeof(superbloque.max_files));
            filename.write(reinterpret_cast<char*>(&superbloque.used_blocks), sizeof(superbloque.used_blocks));
            filename.write(reinterpret_cast<char*>(superbloque.free_map.data()), superbloque.free_map.size() * sizeof(std::size_t));

            for (const auto& inodo : superbloque.TablaInodos) {
                filename.write(reinterpret_cast<const char*>(&inodo), sizeof(Inodos));
            }
        }
        
        }
}
return true; // Archivo eliminado con éxito        
}

bool SystemManager::deleteArchivo(std::string txt) {
    if (ExisteArchivo(txt)) {
        for (int i = 0; i < superbloque.TablaInodos.size(); i++) {
            Inodos& inodo = superbloque.TablaInodos[i];
            std::string nombreArchivo(inodo.nombre, strnlen(inodo.nombre, sizeof(inodo.nombre)));
            if (nombreArchivo == txt) {
                for (int j = 0; j < 64; j++) {
                    inodo.nombre[j] = '\0';
                }

                // Liberar bloques usados
                for (int j = 0; j < 8; j++) {
                    if (inodo.bloques_usados[j] != 0) {
                        superbloque.free_map[inodo.bloques_usados[j]] = 0;
                        inodo.bloques_usados[j] = 0;
                    }
                }

                inodo.ocupado = false;

                // Escribir cambios al archivo
                if (filename.is_open()) {
                    filename.seekp(24);
                    filename.write(reinterpret_cast<char*>(&superbloque.block_count), sizeof(superbloque.block_count));
                    filename.write(reinterpret_cast<char*>(&superbloque.block_size), sizeof(superbloque.block_size));
                    filename.write(reinterpret_cast<char*>(&superbloque.max_files), sizeof(superbloque.max_files));
                    filename.write(reinterpret_cast<char*>(&superbloque.used_blocks), sizeof(superbloque.used_blocks));
                    filename.write(reinterpret_cast<char*>(superbloque.free_map.data()), superbloque.free_map.size() * sizeof(std::size_t));

                    for (const auto& inodo : superbloque.TablaInodos) {
                        filename.write(reinterpret_cast<const char*>(&inodo), sizeof(Inodos));
                    }
                }
                return true; 
            }
        }
        std::cerr << "Error: Archivo no encontrado en la tabla de inodos.\n";
    } else {
        std::cerr << "Error: No se encontró el archivo especificado.\n";
    }
    return false; 
}

bool SystemManager::copyIn(std::string local, std::string externo)
{
    //del sistema al disco simulado
    std::fstream archivopc;
    archivopc.open(externo, std::ios::in | std::ios::out | std::ios::binary);
    if(!archivopc.is_open()){
        std::cerr<<"No se logro abrir correctamente el archivo.\n";
        return true;
    }
    archivopc.seekg(0, std::ios::end);
    std::size_t fileSize = archivopc.tellg();
    std::string dat;
    archivopc.seekg(0);
    archivopc.read(reinterpret_cast<char*>(&dat),fileSize);
    
    return write(dat,local);;
}

bool SystemManager::copyOut(std::string local, std::string externo)
{
    //del disco simulado al sistema

    std::fstream archivo;
    archivo.open(externo, std::ios::in | std::ios::out | std::ios::binary);
    if(!archivo.is_open()){
        std::cerr<<"No se logro abrir el archivo.";
        return false;
    }
    std::string data= showInfoOfDat(local);
    archivo.write(reinterpret_cast<char*>(&data),sizeof(data));
    return true;
}

bool SystemManager::createnewfile(std::string NombreArchivo)
{
    if (!filename.is_open()) {
        std::cerr << "Error: Disco virtual no está abierto.\n";
        return false;
    }
    
    if(!ExisteArchivo(NombreArchivo)){
        // Crear un nuevo inodo
        for (std::size_t i = 0; i < superbloque.TablaInodos.size(); ++i) {
            Inodos& inodo = superbloque.TablaInodos[i];
            if (!inodo.ocupado) {
                std::strncpy(inodo.nombre, NombreArchivo.c_str(), sizeof(inodo.nombre) - 1);
                inodo.nombre[sizeof(inodo.nombre) - 1] = '\0';
                inodo.tamaño = 0;
                inodo.ocupado = true;

                filename.seekp(sizeof(header) + sizeof(superbloque.block_count) + sizeof(superbloque.block_size) +
                               sizeof(superbloque.max_files) + sizeof(superbloque.used_blocks) +
                               superbloque.free_map.size() * sizeof(std::size_t) +
                               i * sizeof(Inodos), std::ios::beg);
                filename.write(reinterpret_cast<const char*>(&inodo), sizeof(Inodos));
                break;
            }
        }
    

    filename.seekp(sizeof(header) + sizeof(superbloque.block_count) + sizeof(superbloque.block_size) +
                   sizeof(superbloque.max_files) + sizeof(superbloque.used_blocks), std::ios::beg);
    filename.write(reinterpret_cast<const char*>(superbloque.free_map.data()),
                   superbloque.free_map.size() * sizeof(std::size_t));

    return true;
    
    }
    return false;
}