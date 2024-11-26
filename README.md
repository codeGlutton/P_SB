# P_SB
The sports simulation game project based Unreal Engine.

## Git Commit Comment
```
type: Title

body(Optional)

Resolves : #issueNo, ...(Solved Issues , Optional)

See also : #issueNo, ...(Ref , Optional)
```
```
Types
+ feat (to add new tool)
+ fix
+ docs
+ style	(to change only code outfit)
+ design (UI)
+ test
+ refactor
+ build
+ ci	(to change config files)
+ perf	(to upgrade performance)
+ rename
+ remove
```

+ Separate title and body with a blank line (/n)
+ Title is used as a statement
+ Describe what you did and why 

## Project Folder Rules
```
Contents : Don't touch
Contents/SpaceBasketball : Project Folder (To prevent folders from merging due to migration)
Contents/SpaceBasketball/SourceArt : AssetFile Folder
Contents/SpaceBasketball/SourceArt/Character : Skeletion Folder
Contents/SpaceBasketball/SourceArt/Animation : Animation Folder
Contents/SpaceBasketball/SourceArt/StaticMesh : StaticMesh Folder
Contents/SpaceBasketball/SourceArt/Art : Texture, Material Folder
Contents/SpaceBasketball/SourceArt/VFX : Niagara, CascadeParticle Folder
Contents/SpaceBasketball/SourceArt/Media : Video Folder
Contents/SpaceBasketball/SourceArt/Sound : BGM, SE Folder
Contents/SpaceBasketball/Maps : Map Folder
Contents/SpaceBasketball/Data : Curve, DataTable, Parameter Folder
Contents/SpaceBasketball/BP : Blueprint Source Folder
Contents/SpaceBasketball/BP/AI : Behabior Tree, BlackBoard, EQS, AI Controller BP Folder
Contents/SpaceBasketball/BP/Anim : Animation BP, AnimMontage Folder
```

## Project Name Rules
Asset Name Rule
```
Prefix_BaseAssetName_Variant_SuffixPermalink
ex)
MainCharacter's 3rd Skeletion's Normal Texture
== T_MainCharSkeltion_03_N
```
Code Prefix Name Rule
```
BP_ : Blueprint Prefix
E : c++ && BP Enum Prefix (※ It is automatically attached in c++)
F : c++ && BP Struct Prefix (※ It is automatically attached in c++)
```
Code Type Name Rule
```
+ UE value is PascalCase
+ Server c++ value is camelCase
+ Bool value prefix is 'b'. Make sure the name is true or false 
ex) bIsCompleted, bIsDead
+ Variable names do not contain class names
ex) UEngine::EngineVersion X -> UEngine::Version O
+ Event, Dispatch name is 'On(Action)'
```
Code Comment Rule
```
Describe

Parameter
Name (in) : Describe
&Name (out) : Describe

Return : Describe
```
