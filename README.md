# :computer: P_SB
<div align="center">
<img src="https://github.com/user-attachments/assets/65e2843b-26ce-48e7-a7bb-ca4d58cf7639" width="600" height="450"/>
</div>

This project is an experimental project that implements the login flow using a IOCP game server and an ASP.NET login server. It was created to grasp the flow of dedicated server and understand the role of each server.  

본 프로젝트는 자체 IOCP 게임 서버와 ASP.NET 로그인 서버를 사용하여 로그인 흐름을 구현한 실험용 프로젝트입니다. Dedicated Server의 흐름을 파악하고 각 서버의 역할을 이해하고자 제작되었습니다.

## :pushpin: Server Architecture (서버 구조)
<div align="center">
<img src="https://github.com/user-attachments/assets/7a1faa5c-5ae8-4f4c-9f75-73e621760ef9" width="600" height="450"/>
</div>

The web server was designed to prevent the game server from being blocked by direct database access during the login process. The cache server was designed to accumulate database update operations and apply them in batches at scheduled intervals, thereby reducing the frequency of database access.  

Web Server는 로그인 과정에서 게임 서버가 직접 DB 대기하는 구조를 회피를 목적으로 구성되었으며, Cache Server는 DB 갱신 데이터를 누적하여 주기마다 일괄적으로 갱신함으로서 DB 접근 빈도를 감소하기 위해 설계되었습니다.

## :wrench: Tools & Technologies (사용한 기술)
- **Game Server** : C++ IOCP
- **Web Server(Login Server)** : ASP.NET Core
- **Cache Server** : Redis
- **Client** : Unreal Engine 5.5
- **Serialize Format** : Protobuf

## :octocat: Git Commit Comment Rule (커밋 규칙)
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

## :open_file_folder: Project Folder Rules (프로젝트 폴더 규칙)
```
Contents : Don't touch
Contents/SB : Project Folder (To prevent folders from merging due to migration)
Contents/SB/SourceArt : AssetFile Folder
Contents/SB/SourceArt/Character : Skeletion Folder
Contents/SB/SourceArt/Animation : Animation Folder
Contents/SB/SourceArt/StaticMesh : StaticMesh Folder
Contents/SB/SourceArt/Art : Texture, Material Folder
Contents/SB/SourceArt/VFX : Niagara, CascadeParticle Folder
Contents/SB/SourceArt/Media : Video Folder
Contents/SB/SourceArt/Sound : BGM, SE Folder
Contents/SB/Map : Map Folder
Contents/SB/Data : Curve, DataTable, Parameter Folder
Contents/SB/Blueprint : Blueprint Source Folder
Contents/SB/Blueprint/AI : Behabior Tree, BlackBoard, EQS, AI Controller BP Folder
Contents/SB/Blueprint/Anim : Animation BP, AnimMontage Folder
```

## :pencil: Naming Rules (이름 규칙)
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
E : Unreal c++ && BP Enum Prefix (※ It is automatically attached in c++)
F : Unreal c++ && BP Struct Prefix (※ It is automatically attached in c++)
```
Code Type Name Rule
```
+ UE value's name follows PascalCase
+ Server c++ value's name follows camelCase

+ Private field prefix is '_'
+ Bool value prefix is 'b'. Make sure the name is true or false 
ex) bIsCompleted, bIsDead
+ Variable names do not contain class names
ex) UEngine::EngineVersion X -> UEngine::Version O
+ Event, Dispatch name is 'On(Action)'

+ Protocol struct and class name follow PascalCase
+ Protocol value's name follows snake_case
```
Code Comment Rule
```
Describe

Parameter
Name (in) : Describe
&Name (out) : Describe

Return : Describe
```
