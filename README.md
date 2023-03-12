## Sample description

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

Where **LCAT** stands for large category and **SCAT** is a small category. **OBJ** is used to declare objects of a small category. Morphisms and functors are indicated by **->** and **=>** respectively.
