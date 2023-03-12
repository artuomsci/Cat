# Cat's sandbox for category theory

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

Where **LCAT** stands for large category and **SCAT** is a small category. **OBJ** is used to declare objects of a small category. Morphisms and functors are indicated by **->** and **=>** respectively. Names of morphisms and functors should be unique in the scope they belong to.

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
