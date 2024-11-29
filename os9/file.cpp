#include "file.h"

file::file() {}

file::file(const std::string& _Name,const std::string& _Data, int _size):name(_Name),Data(_Data), size(_size)
{};
std::string file::getData()const{
    return Data;
}
std::string file::getName()const{
    return name;
}
void file::setName(const std::string& _Name){
    name = _Name;
}
void file::setDate(const std::string& _Data){
    Data =_Data;
}
void file::setSize(int a){
    size = a;
}
int file::getSize(){
    return size;
}
