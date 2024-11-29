#ifndef FILE_H
#define FILE_H
#include <string>
class file
{
public:
    file();
    file(const std::string& _Name,const std::string& _Data, int _size);
    std::string getData()const;
    std::string getName()const;
    int getSize();
    void setName(const std::string& _Name);
    void setDate(const std::string& _Data);
    void setSize(int a);
private:
    std::string Data;
    std::string name;
    int size;
};

#endif // FILE_H
