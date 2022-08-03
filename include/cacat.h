#pragma once

#include <optional>
#include <vector>

#include "cat_export.h"

#include "cat.h"

namespace cat
{
   /**
    * @brief The Func struct represents functors
    */
   struct CAT_EXPORT Func : Arrow
   {
      using FuncName = std::string;

      explicit Func(const Cat::CName& source_, const Cat::CName& target_, const FuncName& name_);
      explicit Func(const Cat::CName& source_, const Cat::CName& target_);

      explicit Func(const Cat& source_, const Cat& target_, const FuncName& name_);
      explicit Func(const Cat& source_, const Cat& target_);

      Func(const Func&) = default;
      Func(Func&&) = default;

      Func& operator  = (Func&&) = default;
      Func& operator  = (const Func&) = default;

      bool operator  < (const Func& func_) const;
      bool operator == (const Func& func_) const;
      bool operator != (const Func& func_) const;

      std::optional<Obj> operator()(const std::optional<Obj>& obj_) const;

      MorphVec morphisms;
   };

   using FuncVec = std::vector<Func>;

   /**
   * @brief Map obj with functor
   * @param func_ - functor to map with
   * @param obj_ - object for mapping
   * @return Mapped object
   */
   CAT_EXPORT std::optional<Obj> MapObject(const std::optional<Func>& func_, const std::optional<Obj>& obj_);

   /**
    * @brief The CACat class represents category of categories
    */

   using CACatFrame = Frame<Cat, CatKeyHasher, Func>;

   class CAT_EXPORT CACat : public CACatFrame
   {
   public:

      CACat() = default;

      CACat(const CACat&) = delete;
      CACat(CACat&&) = delete;

      CACat& operator = (CACat&&) = delete;
      CACat& operator = (const CACat&) = delete;

      /**
         * @brief Add node
         * @param node_ - node
         * @return True if successful
         */
      bool AddNode(const Cat& node_);

      /**
       * @brief Add arrow
       * @param arrow_ - arrow
       * @return True if successful
       */
      bool AddArrow(const Func& arrow_);

      /**
       * @brief Verifying functor
       * @param func_ - functor
       * @return True if successful
       */
      bool Verify(const Func& func_) const;

      /**
       * @brief Create right side of the functor expression
       * @param func_ - functor
       * @return True if successful
       */
      bool Statement(const Func& func_);
   };
}
