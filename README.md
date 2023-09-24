# Cat's sandbox for the category theory

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/cat.png" width="64">

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
      chicken -[isBigger]-> cow {};
      cow -[isSmaller]-> chicken
      {
         /* Functions */
         1 -[*]-> 10 {};
      };
   }

   SCAT People
   {
      OBJ man;
   }

   SCAT Aliens
   {
      OBJ thing;
   }
   
   /* Functors */
   People -[isSuperior]-> Aliens
   {
      /* Morphisms */
      man -[*]-> thing {};
   }
}
```
Where:

| Identifier | Meaning |
| ---      | ---       |
| **LCAT** | large category |
| **SCAT** | small category |
| **OBJ** | object |
| **-[]->** | morphism/functor/function etc. |
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
   
   chicken -[*]-> man -[*]-> alien {};
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
   
   chicken  -[*]-> man {};
   man      -[*]-> alien {};
   chicken  -[*]-> alien {};
}
```
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/smartness_2.png" width="512">

Function **Inverse** does what its name implies i.e. inverses arrows.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/smartness_3.png" width="512">

Now that the chicken can rule the world.

### Function: *Initial/Terminal*

Taking example from psychology:

```
SCAT MaslowHierarchy
{
   OBJ PhysiologicalNeeds, SafetyNeeds, LoveAndBelonging, Esteem, SelfActualization;
   
   PhysiologicalNeeds   -[f0]-> SafetyNeeds {};
   SafetyNeeds          -[f1]-> LoveAndBelonging {};
   LoveAndBelonging     -[f2]-> Esteem {};
   Esteem               -[f3]-> SelfActualization {};
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
   
   0 -[f0]-> 1 -[f1]-> 2 -[f2]-> 3 {};
};
```
<img src="https://github.com/artuomsci/Cat/blob/main/imgs/0-3_sequences.png" width="512">

The return of the function **SolveSequences** is as follows:
```
[0, 1, 2, 3],[0, 1, 3],[0, 2, 3],[0, 3]
```

### Function: *Map*

Given a category or an object we can do mapping using functor or morphism accordingly.

Sample source category:

```
LCAT LargeCategory
{
   SCAT Source
   {
      OBJ a, b;
      a -[f]-> b {};
   };
}
```

Sample functor:

```
LCAT LargeCategory
{
   Source -[F]-> Target
   {
      a -[]-> Fa {};
      b -[]-> Fb {};
   }
}
```
Calling function **Map** will result in new the **Target** category while preserving the structure i.e. objects and morphisms between objects:

```
LCAT LargeCategory
{
   SCAT Target
   {
      OBJ Fa, Fb;
      Fa -[FaFb]-> Fb {};
   };
}
```
### Function: *IsAssociative*

This function checks for associativity. In the example given below, two functors following paths **A -> B -> D** and **A -> C -> D** respectively are not associative.

```
   LCAT LCat
   {
      SCAT A
      {
         OBJ a0, a1;
      }

      SCAT B
      {
         OBJ b0, b1;
      }

      SCAT C
      {
         OBJ c0, c1;
      }

      SCAT D
      {
         OBJ d0, d1;
      }

      A -[*]-> B
      {
         a0 -[*]-> b0 {};
         a1 -[*]-> b1 {};
      }

      B -[*]-> D
      {
         b0 -[*]-> d0 {};
         b1 -[*]-> d1 {};
      }

      A -[*]-> C
      {
         a0 -[*]-> c0 {};
         a1 -[*]-> c1 {};
      }

      C -[*]-> D
      {
         c0 -[*]-> d1 {};
         c1 -[*]-> d0 {};
      }
   }
```
Calling **IsAssociative** on a pair **A -> B -> D** and **A -> C -> D** will return **false**.

### Function: *SolveChoice*

Choice problem can be solved by function **SolveChoice**. Given the following category:

```
LCAT cat
{
   SCAT A
   {
      OBJ a0, a1, a2;
   }

   SCAT B
   {
      OBJ b0, b1;
   }

   SCAT C
   {
      OBJ c0, c1;
   }

   A -[*]-> C
   {
      a0 -[*]-> c0 {};
      a1 -[*]-> c1 {};
      a2 -[*]-> c1 {};
   }

   B -[*]-> C
   {
      b0 -[*]-> c0 {};
      b1 -[*]-> c1 {};
   }
}
```
The result of the function call:

```
   A -[*]-> B
   {
      a0 -[*]-> b0 {};
      a1 -[*]-> b1 {};
      a2 -[*]-> b1 {};
   }
```

### Function: *SolveDetermination*

Determination problem can be solved by function **SolveDetermination**. Given the following category:

```
LCAT cat
{
   SCAT A
   {
      OBJ a0, a1, a2;
   }

   SCAT B
   {
      OBJ b0, b1;
   }

   SCAT C
   {
      OBJ c0, c1;
   }

   A -[*]-> B
   {
      a0 -[*]-> b0 {};
      a1 -[*]-> b1 {};
      a2 -[*]-> b1 {};
   }

   A -[*]-> C
   {
      a0 -[*]-> c0 {};
      a1 -[*]-> c1 {};
      a2 -[*]-> c1 {};
   }
}
```

The result of the function call:

```
   B -[B_C]-> C
   {
      b0 -[*]-> c0 {};
      b1 -[*]-> c1 {};
   }
}
```

## Helper functions

### Function: *QueryArrows*

This function searches for arrows according to a query. Arrows that meet conditions of a query will be returned.
Sample category:

```
SCAT QuerySample
{
   OBJ a, b, c, d;
   a -[*]-> b -[*]-> c {};
   a -[*]-> c {};
};
```
Query examples:

```
QueryArrows("a-[*]->b");
Result:
[a_b]

QueryArrows("a-[*]->*");
Result:
[a_a, a_b, a_c]

QueryArrows("*-[*]->c");
Result:
[c_c, b_c, a_c]

QueryArrows("*-[b_c]->*");
Result:
[b_c]

```

### Function: *QueryNodes*

This function searches for nodes according to a query. Query represents the pattern. Those nodes matching the pattern will be returned as a result. Boolean operations maybe used inside query.
Sample category:

```
SCAT QuerySample
{
   OBJ a, b, c, d;
};
```

Query examples:

```
QueryNodes("*");
Result:
[a, b, c, d]

QueryNodes("a");
Result:
[a]

QueryNodes("a | a");
Result:
[a]

QueryNodes("e | f | a | m | g");
Result:
[a]

QueryNodes("e & f | a | m & g");
Result:
[a]

QueryNodes("a | b | m");
Result:
[a, b]

QueryNodes("a & b & c");
Result:
[a, b, c]

QueryNodes("(a & b) | (c & d)");
Result:
[a, b, c, d]

QueryNodes("~(a & b)");
Result:
[c, d]

```

## Library structure

Library class diagram is presented below. Nodes are used as categories, objects and values. Arrows represent functors, morphisms and functions.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/uml.png" width="512">
