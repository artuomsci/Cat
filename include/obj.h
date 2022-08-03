#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <variant>

#include "cat_export.h"

namespace cat
{
   // Object
   class CAT_EXPORT Obj
   {
   public:

      using TSet = std::variant<int, double, std::string>;

      explicit Obj(const std::string& name_);

      Obj(const Obj& obj_) = default;
      Obj(Obj&& obj_) = default;

      Obj& operator  = (Obj&& obj_) = default;
      Obj& operator  = (const Obj& obj_) = default;

      bool operator  < (const Obj& obj_) const;
      bool operator == (const Obj& obj_) const;
      bool operator != (const Obj& obj_) const;

      /**
       * @brief Return object name
       * @return Name of the object
       */
      const std::string& Name() const;

      /**
       * @brief Return object as a "set"
       * @return "Set" representation
       */
      const TSet& Value() const;

      /**
       * @brief Set object as a "set"
       * @param value_ - "set" representation
       */
      void SetValue(const TSet& value_);

   private:

      std::string m_name;
      TSet        m_set;
   };

   struct CAT_EXPORT ObjKeyHasher
   {
      std::size_t operator()(const Obj& k_) const;
   };

   using ObjSet      = std::set<Obj>;
   using ObjSetPair  = std::pair<Obj, ObjSet>;
   using ObjVec      = std::vector<Obj>;
   using ObjUMap     = std::unordered_map<Obj, ObjSet, ObjKeyHasher>;
}
