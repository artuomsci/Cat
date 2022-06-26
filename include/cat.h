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
   enum class ELogMode
   {
      eQuiet,
      eConsole
   };

   CAT_EXPORT void set_log_mode(ELogMode mode_);
   CAT_EXPORT ELogMode get_log_mode();
   CAT_EXPORT void print_error(const std::string& msg_);
   CAT_EXPORT void print_info(const std::string& msg_);

   // Object
   class CAT_EXPORT Obj
   {
   public:

      explicit Obj(const std::string& name_);
      explicit Obj(const Obj& obj_);

      bool operator==(const Obj& obj_) const;
      bool operator!=(const Obj& obj_) const;
      bool operator<(const Obj& obj_) const;

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
   struct CAT_EXPORT MorphDef
   {
      explicit MorphDef(const Obj& source_, const Obj& target_, const std::string& morph_name_);
      explicit MorphDef(const Obj& source_, const Obj& target_);

      Obj         source;
      Obj         target;
      std::string name;

      bool operator<(const MorphDef& morph_) const;
      bool operator==(const MorphDef& morph_) const;
   };

   using ObjSet      = std::set<Obj>;
   using ObjSetPair  = std::pair<Obj, ObjSet>;
   using ObjUMap     = std::unordered_map<Obj, ObjSet, ObjKeyHasher>;
   using MorphSet    = std::set<MorphDef>;
   using ObjVec      = std::vector<Obj>;

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
      bool AddMorphism(const MorphDef& morph_);

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
       * @return Result of erasure
       */
      bool EraseMorphism(const std::string& morph_name_);

      /**
       * @brief Erase all morphisms
       */
      void EraseMorphisms();

      /**
       * @brief Add object to category
       * @param obj_ - object to add
       * @return Result of adding object
       */
      bool AddObject(const Obj& obj_);

      /**
       * @brief Add object to category
       * @param obj_ - object to add
       * @return Result of adding object
       */
      template <typename T, typename... TArgs>
      bool AddObjects(const T& obj_, const TArgs&... objs_)
      {
         return AddObject(obj_) ? AddObjects(objs_...) : false;
      }

      /**
       * @brief Erase object from category
       * @param obj_ - object to erase
       * @return Result of erasure
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

   // Functor
   struct CAT_EXPORT Func
   {
      using FuncName = std::string;

      explicit Func(const Cat::CatName& source_, const Cat::CatName& target_, const FuncName& name_) :
            source   (source_)
         ,  target   (target_)
         ,  name     (name_)
      {}

      Cat::CatName   source;
      Cat::CatName   target;
      FuncName       name;
      MorphSet       morphisms;

      bool operator < (const Func& func_) const;
   };

   // Category of categories
   class CAT_EXPORT CACat
   {
   public:

      void AddCategory(const Cat& cat_);
      void AddFunctor(const Func& func_);
      const std::set<Cat>& Categories() const;
      const std::set<Func>& Functors() const;
      std::optional<Obj> MapObject(const Func& func_, const Obj& obj_) const;
      bool Validate(Func& func_) const;
      std::optional<Func> MatchFunctor(const Cat::CatName& source_, const Cat::CatName& target_);

   private:
      std::set<Cat> m_cats;
      std::set<Func> m_funcs;
   };
}
