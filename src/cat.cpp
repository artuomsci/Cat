#include "cat.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string& source_, const std::string& target_, const std::string& arrow_name_) :
   source      (source_)
 , target      (target_)
 , name        (arrow_name_)
{};

//-----------------------------------------------------------------------------------------
Arrow::Arrow(const std::string& source_, const std::string& target_) :
   source      (source_)
 , target      (target_)
 , name        (default_arrow_name(source_, target_))
{};

//-----------------------------------------------------------------------------------------
bool Arrow::operator<(const Arrow& arrow_) const
{
   return std::tie(source, target, name) < std::tie(arrow_.source, arrow_.target, arrow_.name);
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator==(const Arrow& arrow_) const
{
   return source == arrow_.source && target == arrow_.target && name == arrow_.name;
}

//-----------------------------------------------------------------------------------------
bool Arrow::operator!=(const Arrow& arrow_) const
{
   return source != arrow_.source || target != arrow_.target || name != arrow_.name;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Morph::Morph(const std::string& source_, const std::string& target_, const std::string& name_) :
   Arrow(source_, target_, name_)
{
}

//-----------------------------------------------------------------------------------------
Morph::Morph(const std::string& source_, const std::string& target_) :
   Arrow(source_, target_, default_arrow_name(source_, target_))
{
}

//-----------------------------------------------------------------------------------------
Morph::Morph(const Obj& source_, const Obj& target_, const std::string& name_) :
   Arrow(source_.Name(), target_.Name(), name_)
{
}

//-----------------------------------------------------------------------------------------
Morph::Morph(const Obj& source_, const Obj& target_) :
   Arrow(source_.Name(), target_.Name(), default_arrow_name(source_.Name(), target_.Name()))
{
}

//-----------------------------------------------------------------------------------------
bool Morph::operator==(const Morph& morph_) const
{
   return source == morph_.source && target == morph_.target && name == morph_.name;
}

//-----------------------------------------------------------------------------------------
bool Morph::operator!=(const Morph& morph_) const
{
   return source != morph_.source || target != morph_.target || name != morph_.name;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
Cat::Cat(const CName& name_) : m_name(name_) {};

//-----------------------------------------------------------------------------------------
bool Cat::operator < (const Cat& cat_) const
{
   return m_name < cat_.m_name;
}

//-----------------------------------------------------------------------------------------
bool Cat::operator ==(const Cat& cat_) const
{
   return m_name == cat_.Name();
}

//-----------------------------------------------------------------------------------------
bool Cat::operator !=(const Cat& cat_) const
{
   return m_name != cat_.Name();
}

//-----------------------------------------------------------------------------------------
const Cat::CName& Cat::Name() const
{
   return m_name;
}

//-----------------------------------------------------------------------------------------
std::size_t CatKeyHasher::operator()(const Cat& k_) const
{
   return std::hash<std::string>{}(k_.Name());
}
