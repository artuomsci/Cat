#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <optional>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>

#include "str_utils.h"

namespace cat
{
   // Keywords
   static const char* const sObj = "obj";
   static const char* const sCat = "cat";

   enum class ELogMode
   {
      eQuiet,
      eConsole
   };

   void set_log_mode(ELogMode mode_);
   ELogMode get_log_mode();
   void print_error(const std::string& msg_);
   void print_info(const std::string& msg_);

   class Obj
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
      const std::string m_name;
   };

   struct ObjKeyHasher
   {
     std::size_t operator()(const Obj& k_) const
     {
       return std::hash<std::string>{}(k_.GetName());
     }
   };

   struct MorphDef
   {
      explicit MorphDef(const Obj& start_, const Obj& end_, const std::string& morph_name_) :
         start       (start_)
       , end         (end_)
       , morph_name  (morph_name_)
      {};

      explicit MorphDef(const Obj& start_, const Obj& end_) :
         start       (start_)
       , end         (end_)
       , morph_name  (start.GetName() + end.GetName())
      {};

      Obj         start;
      Obj         end;
      std::string morph_name;

      bool operator<(const MorphDef& morph_) const
      {
         return std::tie(start, end, morph_name) < std::tie(morph_.start, morph_.end, morph_.morph_name);
      }
   };

   using ObjSet      = std::set<Obj>;
   using ObjSetPair  = std::pair<Obj, ObjSet>;
   using ObjUMap     = std::unordered_map<Obj, ObjSet, ObjKeyHasher>;
   using MorphSet    = std::set<MorphDef>;
   using ObjVec      = std::vector<Obj>;

   class Cat
   {
   public:

      explicit Cat(const std::string& name_);

      /**
       * @brief Add morphism to the category
       * @param start_ - start object
       * @param end_ - end object
       * @param morph_name_ - morphism name
       * @return Result of adding morphism
       */
      bool AddMorphism(const Obj& start_, const Obj& end_, const std::string& morph_name_);

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
      bool AddMorphisms() { return true; }

      template <typename T, typename... TArgs>
      bool AddMorphisms(const T& morph_, const TArgs&... args_)
      {
         return AddMorphism(morph_) ? AddMorphisms(args_...) : false;
      }

      /**
       * @brief Delete morphism
       * @param morph_name_ - morphism name
       * @return Result of deleting morphism
       */
      bool DelMorphism(const std::string& morph_name_);

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
      bool AddObjects() { return true; }

      template <typename T, typename... TArgs>
      bool AddObjects(const T& obj_, const TArgs&... objs_)
      {
         return AddObject(obj_) ? AddObjects(objs_...) : false;
      }

      /**
       * @brief Delete object from category
       * @param obj_ - object to delete
       * @return Result of deleting object
       */
      bool DelObject(const Obj& obj_);

      /**
       * @brief Return category name
       * @return Name of the category
       */
      const std::string& GetName() const;

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

   private:
      const std::string m_name;
      ObjUMap           m_objects;
      MorphSet          m_morphisms;
   };

   /**
    * @brief Parse the contents of string
    * @param source_ - string to parse
    * @param cats_ - resulting categories
    * @return Parsing result
    */
   bool parse_source(const std::string& source_, std::vector<Cat>& cats_);

   /**
    * @brief Load text file into string
    * @param filename_ - path to file
    * @return String with contents of file
    */
   std::optional<std::string> get_description(const std::string& filename_);

   /**
    * @brief Find any sequence of objects between two given objects
    * @param cat_ - category to find sequence of objects in
    * @param from_ - initial object of sequence
    * @param to_ - terminal object of sequence
    * @return Sequence of objects
    */
   ObjVec solve_sequence(const Cat& cat_, const Obj& from_, const Obj& to_);

   /**
    * @brief Find all sequences of objects between two given objects
    * @param cat_ - category to find sequences of objects in
    * @param from_ - initial object of sequences
    * @param to_ - terminal object of sequences
    * @return Sequences of objects
    */
   std::vector<ObjVec> solve_sequences(const Cat& cat_, const Obj& from_, const Obj& to_);

   /**
    * @brief Map sequence of objects onto sequence of morphisms
    * @param objs_ - sequence of objects
    * @param cat_ - category to find morphisms in
    * @return Sequence of morphisms
    */
   StringVec map_obj2morphism(const ObjVec& objs_, const Cat& cat_);

   /**
    * @brief Find all compositions
    * @param cat_ - category to find compositions in
    */
   void solve_compositions(Cat& cat_);

   /**
    * @brief Return identity morphism name
    * @param obj_ - object for identity morphism
    * @return Identity morphism name
    */
   std::string id_morph_name(const Obj& obj_);
}
