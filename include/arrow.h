#pragma once

#include <list>
#include <optional>
#include <string>
#include <vector>

#include "cat_export.h"

namespace cat {
class Node;

/**
 * @brief The Arrow class represents morphisms and functors
 */
class CAT_EXPORT Arrow {
public:
  Arrow(const std::string &source_, const std::string &target_,
        const std::string &arrow_name_);
  Arrow(const std::string &source_, const std::string &target_);

  Arrow(const Node &source_, const Node &target_,
        const std::string &arrow_name_);
  Arrow(const Node &source_, const Node &target_);

  bool operator<(const Arrow &arrow_) const;
  bool operator==(const Arrow &arrow_) const;
  bool operator!=(const Arrow &arrow_) const;

  Arrow(const Arrow &) = default;
  Arrow(Arrow &&) = default;

  Arrow &operator=(Arrow &&) = default;
  Arrow &operator=(const Arrow &) = default;

  using Vec = std::vector<Arrow>;
  using List = std::list<Arrow>;
  using AName = std::string;

  /**
   * @brief Returns default arrow name
   * @param source_ - source name
   * @param target_ - target name
   * @return Arrow name
   */
  static std::string DefaultArrowName(const std::string &source_,
                                      const std::string &target_);

  /**
   * @brief Returns identity arrow name
   * @param name_ - source/target name
   * @return Arrow name
   */
  static std::string IdArrowName(const std::string &name_);

  /**
   * @brief Sets default name
   */
  void SetDefaultName();

  /**
   * @brief Returns arrow source name
   * @return Name
   */
  const std::string &Source() const;

  /**
   * @brief Sets source name
   * @param source_ - source name
   */
  void SetSource(const std::string &source_);

  /**
   * @brief Returns arrow target name
   * @return Name
   */
  const std::string &Target() const;

  /**
   * @brief Sets target name
   * @param target_ - target name
   */
  void SetTarget(const std::string &target_);

  /**
   * @brief Returns arrow name
   * @return Name
   */
  const AName &Name() const;

  /**
   * @brief Adds arrow
   * @param arrow_ - arrow
   */
  void AddArrow(const Arrow &arrow_);

  /**
   * @brief Adds arrow
   * @param args_ - arrow arguments
   */
  template <typename... Args> void EmplaceArrow(Args &&...args_) {
    AddArrow(Arrow(std::forward<Args>(args_)...));
  }

  /**
   * @brief Erases arrow
   * @param arrow_ - arrow name
   */
  void EraseArrow(const Arrow::AName &arrow_);

  /**
   * @brief Erases all arrows
   */
  void EraseArrows();

  /**
   * @brief Queries for arrows
   * Syntax: source name -> target name
   * Syntax for named arrows: source name -[ arrow name ]-> target name
   * Use "*" as a replacement for any name
   * @param query_ - query
   * @return Arrows
   */
  List QueryArrows(
      const std::string &query_,
      std::optional<size_t> matchCount_ = std::optional<size_t>()) const;

  /**
   * @brief Checks whether the arrow contains any arrows
   * @return True if there are no arrows
   */
  bool IsEmpty() const;

  /**
   * @brief Maps internal node
   * @param node_ - node for mapping
   * @return Mapped node
   */
  std::optional<Node> SingleMap(const std::optional<Node> &node_) const;

  /**
   * @brief Maps internal node
   * @param name_ - node name for mapping
   * @return Mapped node
   */
  std::optional<Node> SingleMap(const std::string &name_) const;

  /**
   * @brief Maps node
   * @param name_ - node name for mapping
   * @return Mapped node
   */
  std::optional<Node> Map(const std::optional<Node> &node_) const;

  /**
   * @brief Inverses arrow
   */
  void Inverse();

  /**
   * @brief Cheks for inversion possibility
   * @return True if successful
   */
  bool IsInvertible() const;

  /**
   * @brief Returns arrow as query
   * @return Query representation
   */
  std::string AsQuery() const;

  /**
   * @brief Checks associativity of arrows
   * @param arrow - arrow for associativity check
   * @return True if associative
   */
  bool IsAssociative(const Arrow &arrow) const;

  /**
   * @brief Counts the number of arrows
   * @return Number of arrows
   */
  size_t CountArrows() const;

  /**
   * @brief Validates arrow
   * @return True if successful
   */
  bool IsValid() const;

private:
  std::optional<Node> singleMapImpl(const std::string &name_) const;

  std::string m_source;
  std::string m_target;
  AName m_name;
  List m_arrows;
};

} // namespace cat
