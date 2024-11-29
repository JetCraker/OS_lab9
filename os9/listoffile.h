#ifndef LISTOFFILE_H
#define LISTOFFILE_H
#include "file.h"
#include <vector>

class ListOfFile
{
public:
    ListOfFile();
    void getlist(char* mes);
    file& operator[](size_t a);
    std::vector<file> getlist();
    size_t size() ;
    size_t getSizeOfList();
private:
    std::vector<file> list;
    size_t sizeOfList;
};

#endif // LISTOFFILE_H
