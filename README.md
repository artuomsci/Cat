# Cat
Learning basics of category theory by coding

### Discrete category

```haskell
-- Category
cat Demo

-- Objects
obj a & b & c
obj d
```
Where **cat** is the keyword for category and **obj** is for objects. Multiple objects are separated from each other by **&**.

Names are case sensitive and UTF-8 friendly.

The following symbols may be used in names:

| Symbol | Name   |
| ------ | ------ |
|   a-z  |        |
|   0-9  |        |
|        | Space  |
|    ,   | Comma  |

<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/1.png" width="256" height="256">
</p>

By default, there is always a **large category** containing **small categories** declared by user.

### Defining morphisms

```haskell
-- Category
cat Demo

-- Objects
obj a & b
obj d
obj c & e & f

-- Named morphism
f :: a -> d

-- Anonymous morphism
* :: b -> d
-- Internally, the morphism will be given name after its source and target objects, that is "b-d"

-- Multiple morphisms
* :: c -> e -> f
-- The following morphisms will be created: c -> e, e -> f

```
The morphism name is on the left side of **::** which splits the definition in two parts. The right side represents sequence of morphisms, where  objects are separated with **->**.
