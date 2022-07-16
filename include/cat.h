#pragma once

#include <vector>
#include <optional>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>

#include "cat_export.h"

#include "types.h"

namespace cat
{
   // Object
   class CAT_EXPORT Obj
   {
   public:

      explicit Obj(const std::string& name_);
      explicit Obj(const Obj& obj_);

      Obj(Obj&&) = default;

      void operator= (const Obj& obj_);
      bool operator==(const Obj& obj_) const;
      bool operator!=(const Obj& obj_) const;
      bool operator< (const Obj& obj_) const;

      /**
       * @brief Return object name
       * @return Name of the object
       */
      const std::string& GetName() const;

   private:
      std::string m_name;
   };

   struct CAT_EXPORT ObjKeyHasher
   {
     std::size_t operator()(const Obj& k_) const;
   };

   // Morphism
   struct CAT_EXPORT Morph
   {
      explicit Morph(const Obj& source_, const Obj& target_, const std::string& morph_name_);
      explicit Morph(const Obj& source_, const Obj& target_);

      Obj         source;
      Obj         target;
      std::string name;

      bool operator  <  (const Morph& morph_) const;
      bool operator  == (const Morph& morph_) const;
   };

   using MorphSet    = std::set<Morph>;

   using ObjSet      = std::set<Obj>;
   using ObjSetPair  = std::pair<Obj, ObjSet>;
   using ObjUMap     = std::unordered_map<Obj, ObjSet, ObjKeyHasher>;
   using ObjVec      = std::vector<Obj>;

   //-----------------------------------------------------------------------------------------
   enum class EExpType
   {
         eStatement
      ,  eProof
   };

   // Category
   class CAT_EXPORT Cat
   {
   public:

      using CatName = std::string;

      explicit Cat(const CatName& name_);

      bool operator < (const Cat& cat_) const;

      /**
       * @brief Add morphism to the category
       * @param source_ - source object
       * @param target_ - target object
       * @param morph_name_ - morphism name
       * @return Result of adding morphism
       */
      bool AddMorphism(const Obj& source_, const Obj& target_, const std::string& morph_name_);

      /**
       * @brief Add morphism to the category
       * @param morph_ morphism to add
       * @return Result of adding morphism
       */
      bool AddMorphism(const Morph& morph_);

      /**
       * @brief Add morphism to the category
       * @param morph_ morphism to add
       * @return Result of adding morphism
       */
      template <typename T, typename... TArgs>
      bool AddMorphisms(const T& morph_, const TArgs&... args_)
      {
         return AddMorphism(morph_) ? AddMorphisms(args_...) : false;
      }

      /**
       * @brief Erase morphism
       * @param morph_name_ - morphism name
       * @return True if successfully erased
       */
      bool EraseMorphism(const std::string& morph_name_);

      /**
       * @brief Erase all morphisms
       */
      void EraseMorphisms();

      /**
       * @brief Add object to the category
       * @param obj_ - object to add
       * @return True if successfully added
       */
      bool AddObject(const Obj& obj_);

      /**
       * @brief Add objects to the category
       * @param obj_ - object to add
       * @return Result of adding object
       */
      template <typename T, typename... TArgs>
      bool AddObjects(const T& obj_, const TArgs&... objs_)
      {
         return AddObject(obj_) ? AddObjects(objs_...) : false;
      }

      /**
       * @brief Erase object from the category
       * @param obj_ - object to erase
       * @return True if successfully erased
       */
      bool EraseObject(const Obj& obj_);

      /**
       * @brief Erase all objects
       */
      void EraseObjects();

      /**
       * @brief Return category name
       * @return Name of the category
       */
      const CatName& GetName() const;

      /**
       * @brief Return category morphisms
       * @return Morphisms
       */
      const MorphSet& GetMorphisms() const;

      /**
       * @brief Return category objects
       * @return Objects
       */
      const ObjUMap& GetObjects() const;

      /**
      * @brief Checking for morphism
      * @param source_ - source object
      * @param target_ - target object
      * @return True if morphism exists
      */
      bool MatchMorphism(const Obj& source_, const Obj& target_) const;

   private:
      // terminal condition
      bool AddMorphisms() { return true; }
      // terminal condition
      bool AddObjects() { return true; }

      CatName     m_name;
      ObjUMap     m_objects;
      MorphSet    m_morphisms;
   };

   using CatSet = std::set<Cat>;
}
