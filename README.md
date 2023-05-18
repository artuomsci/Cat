# Cat's sandbox for the category theory

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/cat.png" width="512">

## Syntax example

```
/* Large category definition */
LCAT World
{
   /* Small category definition */
   SCAT Animals
   {
      /* Objects */
      OBJ cow, chicken;
      
      /* Morphisms */
      chicken -[isBigger]-> cow -[isSmaller]-> chicken;
   }

   SCAT People;
   SCAT Aliens;
   
   /* Functors */
   People =[isSuperior]=> Aliens =[*]=> People;
};
```
Where:

| Identifier | Meaning |
| ---      | ---       |
| **LCAT** | large category |
| **SCAT** | small category |
| **OBJ** | object |
| **-[]->** | morphism |
| **=[]=>** | functor |
| * | hint for auto-generating morphism/functor name |
| **/\*...\*/** | comment section |

Names of morphisms and functors should be unique in the scope they are defined.

## Features and example applications 
### Function: *SolveCompositions*

Let's have a look at intelligence relations between representatives of different species.
```
SCAT Creatures
{
   OBJ alien, man, chicken;
   
   chicken -[*]-> man -[*]-> alien;
}
```
Directed graph is shown below.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/smartness_1.png" width="512">

By applying function **SolveCompositions** we get the following result:

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/smartness_2.png" width="512">

By the rule of composition follows the conclusion that alien is smarter than chicken, that is: chicken -> alien.

### Function: *Inverse*

Suppose we have the following category where alien is the most intelligent creature:

```
SCAT Creatures
{
   OBJ alien, man, chicken;
   
   chicken  -[*]-> man;
   man      -[*]-> alien;
   chicken  -[*]-> alien;
}
```
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/smartness_2.png" width="512">

Function **Inverse** does what its name implies.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/smartness_3.png" width="512">

Now that the chicken can rule the world.

### Function: *Initial/Terminal*

Taking example from psychology:

```
SCAT MaslowHierarchy
{
   OBJ PhysiologicalNeeds, SafetyNeeds, LoveAndBelonging, Esteem, SelfActualization;
   
   PhysiologicalNeeds   -[f0]-> SafetyNeeds;
   SafetyNeeds          -[f1]-> LoveAndBelonging;
   LoveAndBelonging     -[f2]-> Esteem;
   Esteem               -[f3]-> SelfActualization;
};
```

Maslow's hierarchy of needs is not so obvious in the picture below. It may seem chaotic at first, but the law has been preserved. The structure has only been equiped with compositions.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/MaslowHierarchy.png" width="512">

Finding the bottom and top of hierarchy obliges us to chase the arrows. However, the same result could be obtained by identifying initial and terminal objects of the category.

The functions **Initial** and **Terminal** return respectively:
```
[PhysiologicalNeeds]
```
```
[SelfActualization]
```

### Function: *SolveSequences*

The problem is to find all combinations of morphisms from object **0** to object **3** given initially **0 -> 1**, **1 -> 2** and **2 -> 3**.
```
SCAT Sum
{
   OBJ 0, 1, 2, 3;
   
   0 -[f0]-> 1 -[f1]-> 2 -[f2]-> 3;
};
```
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/0-3_sequences.png" width="512">

The return of the function **SolveSequences** is as follows:
```
[0, 1, 2, 3],[0, 1, 3],[0, 2, 3],[0, 3]
```

### Function: *QueryNodes*

```
SCAT QuerySample
{
   OBJ a, b, c, d;
};

QueryNodes("*")
Result:
[a, b, c, d]

QueryNodes("a")
Result:
[a]

QueryNodes("a | a")
Result:
[a]

QueryNodes("e | f | a | m | g")
Result:
[a]

QueryNodes("e & f | a | m & g")
Result:
[a]

QueryNodes("a | b | m")
Result:
[a, b]

QueryNodes("a & b & c")
Result:
[a, b, c]

QueryNodes("(a & b) | (c & d)")
Result:
[a, b, c, d]

QueryNodes("~(a & b)")
Result:
[c, d]

```

## Library structure

Library class diagram is presented below. Nodes are used as categories, objects and values. Arrows represent functors, morphisms and functions.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/uml.png" width="512">
