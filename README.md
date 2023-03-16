# Cat's sandbox for the category theory

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/cat.png" width="512">

## Syntax sample

```
LCAT World
{
   SCAT Animals
   {
      OBJ cow, chicken;
      
      isBigger : chicken -> cow;
      isSmaller : cow -> chicken;
   }
   
   SCAT People;
   SCAT Aliens;
   
   isSuperior : People => Aliens;
};
```

Where **LCAT** stands for large category and **SCAT** is a small category. **OBJ** is used to declare objects. Morphisms and functors are indicated by **->** and **=>** respectively. Names of morphisms and functors should be unique in the scope they belong to.

## Examples 
### Function: *SolveCompositions*

Let's have a look at intelligence relations between representatives of different species.
```
SCAT Creatures
{
   OBJ alien, man, chicken;
   
   isSmarter_0 : chicken -> man;
   isSmarter_1 : man -> alien;
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
   
   isSmarter_0 : chicken -> man;
   isSmarter_1 : man -> alien;
   isSmarter_2 : chicken -> alien;
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
   
   f0 : PhysiologicalNeeds -> SafetyNeeds;
   f1 : SafetyNeeds -> LoveAndBelonging;
   f2 : LoveAndBelonging -> Esteem;
   f3 : Esteem -> SelfActualization;
};
```

Maslow's hierarchy of needs is not so obvious in the picture below. It may seem chaotic at first, but the law has been preserved. The structure has only been equiped with compositions.

<img src="https://github.com/artuomsci/Cat/blob/main/imgs/MaslowHierarchy.png" width="512">

Finding the bottom and top of hierarchy obliges us to chase the arrows for some time. However, the same result could be obtained by identifying initial and terminal objects of the category.

Function **Initial** returns the object **PhysiologicalNeeds**. Function **Terminal** returns the object **SelfActualization**.
