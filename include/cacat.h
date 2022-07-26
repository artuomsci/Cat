#pragma once

#include <optional>
#include <vector>

#include "cat_export.h"

#include "cat.h"

namespace cat
{
   // Functor
   struct CAT_EXPORT Func
   {
      using FuncName = std::string;

      explicit Func(const Cat::CatName& source_, const Cat::CatName& target_, const FuncName& name_);
      explicit Func(const Cat::CatName& source_, const Cat::CatName& target_);
      explicit Func(const Cat& source_, const Cat& target_, const FuncName& name_);
      explicit Func(const Cat& source_, const Cat& target_);

      Func(const Func& func_) = default;
      Func(Func&& func_) = default;

      Func& operator  = (Func&& func_) = default;
      Func& operator  = (const Func& func_) = default;

      bool operator  < (const Func& func_) const;
      bool operator == (const Func& func_) const;
      bool operator != (const Func& func_) const;

      Cat::CatName   source;
      Cat::CatName   target;
      FuncName       name;
      MorphVec       morphisms;
   };

   using FuncVec = std::vector<Func>;

   /**
   * @brief Map obj with functor
   * @param func_ - functor to map with
   * @param obj_ - object for mapping
   * @return Mapped object
   */
   CAT_EXPORT std::optional<Obj> MapObject(const std::optional<Func>& func_, const std::optional<Obj>& obj_);

   // Category of categories
   class CAT_EXPORT CACat
   {
   public:

      /**
       * @brief Add category to the category
       * @param cat_ - category to add
       * @return True if successfully added
       */
      bool AddCategory(const Cat& cat_);

      /**
       * @brief Erase category from the category
       * @param cat_ - category to erase
       * @return True if successfully erased
       */
      bool EraseCategory(const Cat& cat_);

      /**
       * @brief Add functor to the category
       * @param func_ - functor to add
       * @param type_ - type of operation
       * @return True if successfully added
       */
      bool AddFunctor(Func func_, EExpType type_);

      /**
       * @brief Erase functor
       * @param func_ - functor to erase
       * @return True if successfully erased
       */
      bool EraseFunctor(const Func& func_);

      /**
       * @brief Return categories
       * @return Categories
       */
      const CatUMap& Categories() const;

      /**
       * @brief Return functors
       * @return Functors
       */
      const FuncVec& Functors() const;

      /**
       * @brief Functor validation
       * @param func_ - functor to validate
       * @return True if functor is valid
       */
      bool Proof(const Func& func_) const;

      /**
       * @brief Create right side of the functor expression
       * @param func_ - functor expression
       * @return True if operation is successfull
       */
      bool Statement(const Func& func_);

      /**
       * @brief Find functor by source and target categories
       * @param source_ - source category
       * @param target_ - target category
       * @return Functor
       */
      std::optional<Func> FindFunctor(const Cat::CatName& source_, const Cat::CatName& target_) const;

      /**
       * @brief Find source categories for the target category
       * @param target_ - target category
       * @return Source categories
       */
      CatNameVec FindSources(const Cat::CatName& target_) const;

      /**
       * @brief Find target categories for the source category
       * @param source_ - source categories
       * @return Target categories
       */
      CatNameVec FindTargets(const Cat::CatName& source_) const;

      /**
      * @brief Find categories with matching targets
      * @param targets_ - target categories
      * @return Categories with matching targets
      */
      CatVec FindByTargets(const CatNameVec& targets_) const;

   private:

      void eraseInstances(const Cat& cat_);

      CatUMap  m_cats;
      FuncVec  m_funcs;
   };
}
