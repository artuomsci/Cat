#include "cat.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Obj::Obj(const std::string& name_) : m_name(name_) {};

//-----------------------------------------------------------------------------------------
bool Obj::operator<(const Obj& obj_) const
{
   return m_name < obj_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Obj::operator==(const Obj& obj_) const
{
   return m_name == obj_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Obj::operator!=(const Obj& obj_) const
{
   return m_name != obj_.m_name;
}

//-----------------------------------------------------------------------------------------
const std::string& Obj::GetName() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
std::size_t ObjKeyHasher::operator()(const Obj& k_) const
{
   return std::hash<std::string>{}(k_.GetName());
}
