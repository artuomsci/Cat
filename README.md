# Cat
Learning basics of category theory by coding

### Sample discrete category

```haskell
-- Category
cat Demo

-- Objects
obj a, b, c, d
```
<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/1.png" width="256" height="256">
</p>

### Finding compositions of morphisms

The goal is to find all possible compositions in the given category:

```haskell
-- Category
cat Demo

-- Objects
obj a, b, c, d

-- Morphisms
f0 :: a -> b
f1 :: b -> c
f2 :: c -> d
```
<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/2.png" width="256" height="256">
</p>
Calling function *solve_compositions* gives us the following result:
<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/3.png" width="256" height="256">
</p>

### Finding sequences of intermediate objects between two given objects

Let's find a set of objects/morphisms leading from one object to another.

```haskell
-- Category
cat Demo

-- Objects
obj a, b, c, d, e, f

-- Morphisms
f0 :: a -> b
f1 :: b -> a

f2 :: b -> d
f3 :: d -> e
f4 :: a -> c
f5 :: c -> d
f6 :: c -> f
f7 :: f -> e
```
<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/4.png" width="256" height="256">
</p>
Calling function *solve_sequences* gives us the following result:

<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/5.png" width="768" height="256">
</p>

### Morphism inversion

Example of finding opposite category

```haskell
-- Category
cat Demo

-- Objects
obj a, b, c, d

-- Morphisms
f0 :: a -> b
f1 :: a -> c

f2 :: b -> d
f3 :: c -> d
```
<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/6.png" width="256" height="256">
</p>

We can get the opposite category by *inversion* :

<p align="center">
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/7.png" width="256" height="256">
</p>
