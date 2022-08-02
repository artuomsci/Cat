#pragma once

#include <vector>

#include "cat_export.h"

#include "frame.h"
#include "obj.h"

namespace cat
{
   /**
    * @brief The Morph struct represents morphisms
    */
   struct CAT_EXPORT Morph : Arrow
   {
      explicit Morph(const std::string& source_, const std::string& target_, const std::string& name_);
      explicit Morph(const std::string& source_, const std::string& target_);

      explicit Morph(const Obj& source_, const Obj& target_, const std::string& name_);
      explicit Morph(const Obj& source_, const Obj& target_);

      bool operator == (const Morph& morph_) const;
      bool operator != (const Morph& morph_) const;
   };

   using MorphVec = std::vector<Morph>;

   /**
    * @brief The Cat class represents categories
    */

   using CatFrame = Frame<Obj, ObjKeyHasher, Morph>;

   class CAT_EXPORT Cat : public CatFrame
   {
   public:

      using CatName = std::string;

      explicit Cat(const CatName& name_);

      bool operator  < (const Cat& cat_) const;
      bool operator == (const Cat& cat_) const;
      bool operator != (const Cat& cat_) const;

      /**
       * @brief Return category name
       * @return Name of the category
       */
      const CatName& GetName() const;

   private:

      CatName m_name;
   };

   struct CAT_EXPORT CatKeyHasher
   {
      std::size_t operator()(const Cat& k_) const;
   };

   using CatSet      = std::set<Cat>;
   using CatSetPair  = std::pair<Cat, CatSet>;
   using CatVec      = std::vector<Cat>;
   using CatNameVec  = std::vector<Cat::CatName>;
   using CatUMap     = std::unordered_map<Cat, CatSet, CatKeyHasher>;
}
