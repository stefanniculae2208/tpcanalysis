#ifndef ErrorCodesMap_hpp
#define ErrorCodesMap_hpp 1

#include <map>
#include <string>


static const std::map<int, std::string> ErrorCodesMap
{

    {0, "Success!"},
    {-1, "File is not open."},
    {-3, "File does not contain the tree."},
    {-3, "Invalid size."},
    {-4, "There exists unassigned data."}

};



#endif
