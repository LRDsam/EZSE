# EZSE

Een eigen game- en rendering-engine in C++20 met OpenGL 4.6, gebouwd als **leerproject**.
Het doel is leren hoe een engine in elkaar zit en de onderdelen zo opzetten dat ze los
herbruikbaar zijn (bijvoorbeeld alleen de Renderer in een ander project). Snelheid van
opleveren is geen doel.

Dit bestand is de overdracht tussen sessies en machines. Werk de secties **Huidige stand**
en **Volgende stap** bij aan het eind van elke afgeronde stap.

## Werkwijze (belangrijk)

De gebruiker schrijft in het Nederlands; antwoord in het Nederlands. Code, commentaar in
code en commitberichten zijn Engels. Laat programmeertermen in de Nederlandse tekst in het
Engels staan (loop, function, struct, member, header, array, index, pointer, default value);
vertalingen als "lus" werken verwarrend.

- **Elke keuze wordt eerst besproken.** Ook kleine (een naam, een versie, waar een bestand
  staat). Leg de opties voor met de gevolgen en een advies; de gebruiker beslist. Kies nooit
  stilzwijgend een standaard.
- **Eén klein onderwerp per stap.** Wacht op de gebruiker voordat je doorgaat. Nooit meerdere
  modules of bestanden in één keer opzetten.
- **Niets toevoegen zonder concrete aanleiding.** Elke toevoeging moet een probleem oplossen
  dat er nu is. Tijdelijke workarounds worden liever overgeslagen dan ingebouwd.
- **Rendering- en OpenGL-code schrijft de gebruiker zelf.** Geef de opdracht, de
  functiesignaturen met uitleg van de parameters, de achtergrond en eventueel denkvragen,
  maar niet de uitwerking. Geef een hint als de gebruiker vastloopt. Loop de code na als
  daarom gevraagd wordt: lees de bestanden, bouw, en benoem wat goed is, wat fout is en wat
  beter kan, bij voorkeur met een vraag in plaats van de oplossing.
- **Nieuwe files maakt Claude altijd zelf aan**, zonder dat erom gevraagd hoeft te worden:
  de lege header en `.cpp` met `#pragma once`, de includes en `namespace ezse`, en de regels
  in `CMakeLists.txt`. Daarna opnieuw genereren en bouwen. De inhoud (structs, functions)
  schrijft de gebruiker.
- **Ander routinewerk mag Claude doen als de gebruiker erom vraagt** (een hulpfunctie, tests
  uitschrijven, opschonen). Laat daarna zien wat er is aangemaakt.
- **Infrastructuur** (CMake, logging, venster): eerst de volledige code met uitleg tonen,
  na akkoord aanmaken en bouwen.
- **Controleren:** Claude bouwt (Debug en Release) en meldt het resultaat. Het programma
  starten doet de gebruiker zelf, omdat het een venster opent dat blijft staan. Zeg eerlijk
  wat wel en niet gecontroleerd is.
- **Commits:** één per afgeronde stap. Stel het bericht voor en commit pas na akkoord.
  Voeg alleen de bestanden van die stap toe. Geen `Co-Authored-By`-regel in het bericht.
- **Visual Studio:** de gebruiker werkt in `build/EZSE.sln`. Bestanden die Claude wijzigt
  terwijl ze daar open staan, kunnen bij herladen overschreven worden. Vraag de gebruiker
  eerst alles op te slaan (Ctrl+Shift+S) en meld welke bestanden zijn aangepast.

De gebruiker heeft enige ervaring met C++; de basis van de taal hoeft niet uitgelegd, maar
minder gangbare onderdelen wel (templates, `std::span`, macro's, linkage en dergelijke).

## Bouwen en starten

Nodig: Visual Studio 2022 met de C++-workload, CMake 3.24 of nieuwer, git, en een videokaart
met driver die OpenGL 4.6 ondersteunt. De eerste keer genereren downloadt GLFW 3.4 van GitHub.

```bash
cmake -S . -B build
cmake --build build
./build/Sandbox/Debug/Sandbox.exe
./build/Math/Debug/MathTests.exe
./build/ECS/Debug/ECSTests.exe
./build/Engine/Debug/EngineTests.exe
```

`MathTests`, `ECSTests` en `EngineTests` openen geen venster, printen per controle `[PASS]` of
`[FAIL]` en eindigen met exit code `0` als alles klopt. Claude draait ze zelf na elke wijziging
in `Math`, `ECS` of `Engine`. `EngineTests` test alleen wat zonder venster kan.

Release: `cmake --build build --config Release`. De map `build/` staat niet in git.
In Visual Studio: open `build/EZSE.sln`. Bij een nieuw gegenereerde `build/` is Sandbox al
het startproject (`VS_STARTUP_PROJECT`). In een map die al een `.vs/` heeft stel je dat
eenmalig met de hand in (rechtsklik op Sandbox, *Set as Startup Project*); anders geeft F5
"Unable to start program ... ALL_BUILD, toegang geweigerd".

## Architectuur

Aparte statische libraries met afhankelijkheden in één richting. De build dwingt de grenzen af.

```
                       Sandbox
                          │
                       Engine ◄── EngineTests
              /        /     \        \
      Platform   Renderer     ECS ◄── ECSTests
        |   \    /   |   \     |
      GLFW   Core    glad Math ◄── MathTests
              ▲               
              └──── (ECS, Platform en Renderer hangen ook van Core af)

      Niets hangt van Engine af; Sandbox linkt alleen EZSE::Engine.
```

| Module | Bevat | Hangt af van |
|---|---|---|
| `Core` | logging (`LogInfo` enz.), `EZSE_ASSERT`, `ReadTextFile` | niets |
| `Math` | `Mat4` met `operator*` (maal `Mat4` en maal `Vec4`), `Vec3` met `operator+`, unary `operator-` en `operator*` (maal `float`), `Length` en `Normalize`, `Vec4`; `Translation`, `Scale`, `RotationX`, `RotationY`, `RotationZ`, `Perspective` en `TransformDirection` (in `Transform.h`); testprogramma `MathTests` in `Math/tests/` | niets |
| `ECS` | `World` (de ingang: `CreateEntity`, `DestroyEntity`, `IsAlive`, `RegisterComponent`, `AddComponent`, `RemoveComponent`, `GetComponent`, `HasComponent`, `AddSystem`, `Update`); `Entity` en `EntityManager` (`CreateEntity`, `DestroyEntity`, `IsAlive`, `GetCreatedCount`); `ComponentArray<T>` (`Add`, `Remove`, `Get`, `Has`) met basisklasse `IComponentArray`; `ComponentType<T>()`; `ComponentManager` (`Register`, `Add`, `Remove`, `Get`, `Has`, `EntityDestroyed`, `GetSignature`); `Signature` en `c_maxComponentTypes`; `EntitySet` (`Insert`, `Remove`, `Contains`, `GetEntities`); `MakeSignature<Ts...>()`; basisklasse `System` met pure virtual `Update` en `SystemManager` (`Add`, `EntitySignatureChanged`, `EntityDestroyed`, `Update`); testprogramma `ECSTests` in `ECS/tests/` | Core (PUBLIC, want `ComponentArray.h` gebruikt `EZSE_ASSERT`) |
| `Engine` | `Engine` (`Create`, `Run`, `Quit`, `GetWorld`, `GetWindow`, `GetRenderer`): bezit venster, Renderer en `World` en draait de game loop; ingebouwde components in `Components/` (door de `Engine` zelf geregistreerd): `Transform` met `ModelMatrix` en `ViewMatrix`, `MeshRenderer`, `Camera`; systems in `Systems/`: `CameraSystem` en `RenderSystem`; testprogramma `EngineTests` in `Engine/tests/` | Platform, Renderer, ECS (PUBLIC), Core (PRIVATE) |
| `Platform` | `Window` (GLFW, verborgen achter eigen interface) met `IsKeyDown`, `GetMouseDelta` en `SetCursorCaptured`, `WindowEvent` (`FramebufferResized`, `KeyPressed`, `KeyReleased`) en `Key` | Core, GLFW (PRIVATE) |
| `Renderer` | OpenGL laden, meshes, shaders, tekenen | Math (PUBLIC), Core, glad (PRIVATE) |
| `Sandbox` | testprogramma: het "spel" dat de engine gebruikt | Engine |
| `ThirdParty` | GLFW via FetchContent; gegenereerde glad-bestanden in `glad/` | |

Regels die de Renderer herbruikbaar houden:

1. **Alle OpenGL-aanroepen blijven binnen `Renderer`.** Geen enkele publieke header include
   glad of gebruikt `GL`-types (in headers `unsigned int` in plaats van `GLuint`).
2. **Renderer en Platform kennen elkaar niet.** Sandbox geeft de laadfunctie van het venster
   door aan `Renderer::Create`. Beide modules declareren zelf het type `GLLoadFunction`.
3. **De Renderer kent geen bestanden, entities of scenes.** Hij krijgt ruwe gegevens en geeft
   handles terug.
4. **Over modulegrenzen gaan handles, geen pointers.** De Renderer slaat de GPU-objecten op
   en verwijdert ze op verzoek (`Destroy...`) en bij afsluiten. Wie `Create...` aanroept
   beslist over de levensduur; later wordt dat een resource manager in de Engine-module,
   en ECS-componenten bevatten alleen handles.

## Conventies

- Namespace `ezse`; macro's met voorvoegsel `EZSE_`; CMake-targets `EZSECore` met alias
  `EZSE::Core`; bestandsnamen met hoofdletter (`Window.h`).
- Per module `include/EZSE/<Module>/` (publiek) en `src/` (intern). Includes met het volledige
  pad: `#include "EZSE/Core/Log.h"`. Eigen headers boven standaardheaders.
- `#pragma once`. Membervariabelen met `m_`. Constanten op namespace-niveau met `c_`
  (`c_cameraBindingPoint`); een lokale `const` in een function krijgt geen voorvoegsel.
  Geen hoofdletters met underscores: die zijn voor macro's.
- Een module met meerdere soorten bestanden krijgt submappen per soort, zowel in `include/` als
  in `src/` (in `Engine`: `Components/`, straks `Systems/`). Modules met één soort blijven plat.
- Hulpfuncties die alleen in één `.cpp` nodig zijn staan in een naamloze namespace.
- Een macro alleen als een functie het niet kan (asserts, platform-schakelaars).
- Iets dat kan mislukken krijgt een `Create`-functie die een lege `unique_ptr` of een handle
  met id `0` teruggeeft en de oorzaak logt. Geen exceptions.
- Asserts zijn voor programmeerfouten; situaties die bij een gebruiker kunnen voorkomen
  (ontbrekend bestand, shader met typfout) krijgen gewone foutafhandeling.
- Elke nieuwe header of `.cpp` wordt bijgeschreven in de `CMakeLists.txt` van de module.
- OpenGL in Direct State Access-stijl (`glCreate...`, `glNamed...`, `glVertexArray...`).
  Bij uitleg ook de klassieke vorm tonen, omdat tutorials die gebruiken.
- Handles: een struct per soort met een `std::uint32_t id`, `0` is ongeldig. Opslag in een
  `std::vector`, id is de index plus één. Verwijderde plekken blijven leeg en worden niet
  hergebruikt.

## Genomen besluiten

| # | Onderwerp | Besluit |
|---|---|---|
| 1–3 | Taal en build | C++20, CMake, MSVC |
| 4 | Indeling | elke module een eigen map en library; `include/` en `src/` gescheiden |
| 5 | Versiebeheer | git, branch `main`, één commit per stap |
| 6 | Soort library | statisch |
| 7 | Logging | zelf geschreven: niveaus en `std::format`; functies, geen macro's; alles naar `std::cout` |
| 8–11 | Venster | GLFW 3.4 via FetchContent in `ThirdParty/`; `GLFWwindow` vooraf gedeclareerd in `Window.h` |
| 12 | Foutafhandeling | `Create`-functies, geen exceptions |
| 13 | OpenGL-versie | 4.6 core profile |
| 14–15 | Loader | glad 2, één keer gegenereerd op gen.glad.sh (gl 4.6 core, geen extensies, alle opties uit) en vastgelegd in git |
| 16 | Laden van OpenGL | de opvraagfunctie wordt aan de Renderer aangereikt |
| 17 | Debug-uitvoer | debug context en callback alleen in Debug-builds (`EZSE_DEBUG`); alleen loggen, niet pauzeren |
| 19 | Stijl | Direct State Access |
| 20 | Venstergrootte | uitgesteld tot er een event-systeem is; geen tijdelijke oplossing. Opgelost in stap 16 (besluiten 58 tot en met 62) |
| 21–23 | Resources | handles; Renderer slaat GPU-objecten op, resource manager komt later in Engine |
| 24 | Hoekpunten | één vast `Vertex`-formaat (positie op attribuut 0, kleur op 1, en sinds besluit 138 de normaal op 2); een vrij formaat ontwerpen we bij het eerste tweede formaat, waarschijnlijk texturen |
| 25 | Indices | altijd verplicht, 32 bits (`std::uint32_t`); `CreateMesh` controleert met asserts dat het aantal een veelvoud van drie is en dat elke index naar een bestaand hoekpunt wijst |
| 26 | Wiskunde | zelf geschreven, geen glm. De gebruiker schrijft de code zoals bij rendering: Claude geeft opdracht, signaturen en achtergrond en loopt na. Elke functie controleren met bekende uitkomsten voordat hij de shader in gaat |
| 27 | Module `Math` | eigen statische library `EZSE::Math` met `.cpp`-bestanden (niet header-only); hangt van niets af, `Core` komt erbij bij de eerste `EZSE_ASSERT` |
| 28 | Namen in `Math` | `Mat4` en `Vec3`, zoals in GLSL. Begonnen met `Mat4`; `Vec3` komt zodra een functie hem nodig heeft |
| 29 | Indeling `Mat4` | kolom voor kolom (column-major), zoals OpenGL en glm; lid `float m[4][4]` met index `[kolom][rij]`. De verplaatsing staat in `m[3]` |
| 30 | Beginwaarde `Mat4` | de eenheidsmatrix, als beginwaarde van het lid in de struct zelf; een `Mat4` is daardoor nooit ongeldig |
| 31 | `Vec4` | nog niet. `Mat4` blijft `float m[4][4]` en wordt niet uit vier `Vec4`-kolommen opgebouwd. Opnieuw bekijken bij de eerste functie die een punt transformeert (matrix maal vector van vier); overstappen verandert de indeling in het geheugen niet |
| 32 | Vermenigvuldigen | `Mat4 operator*(const Mat4& a, const Mat4& b)`, een losse function in `Mat4.h`/`Mat4.cpp` |
| 33 | Volgorde | kolomvectoren, zoals OpenGL, GLSL en glm: `A * B` past eerst B toe en dan A; een punt transformeer je als `M * p`. Een keten lees je van rechts naar links |
| 34 | Controleren van wiskunde | een eigen testprogramma `MathTests` (geen testbibliotheek, geen CTest) met `Check` en `NearlyEqual`; per onderwerp een function `Test...`. Verwachte matrices worden in de tests met de hand opgebouwd (`MakeTranslation`, `MakeScale`), zodat de tests niet leunen op de code die ze controleren. Performance meten hoort in een apart programma, later |
| 35 | `Vec3` | struct met de members `x`, `y`, `z` (elk `float`, default value `0.0f`), in een eigen header `Vec3.h`. Geen array, geen `union` met `r`, `g`, `b` |
| 36 | Transformatie-functions | maken een nieuwe matrix en combineren niets: `Mat4 Translation(const Vec3& offset)`. Combineren doet de aanroeper met `operator*`, zodat de volgorde zichtbaar is. Namen zijn zelfstandige naamwoorden (`Translation`, `Scale`, `RotationX`). `Scale` neemt een `Vec3` met een factor per as |
| 37 | Plek van die functions | `Transform.h` en `Transform.cpp`, zodat `Mat4.h` klein blijft en niet van `Vec3` afhangt |
| 38 | Matrix naar de shader | een matrix per aanroep: `DrawMesh(mesh, shader, const Mat4& transform)`. Geen aparte function om een uniform te zetten |
| 39 | Renderer en `Math` | de Renderer kent `Mat4` en linkt `EZSE::Math` als `PUBLIC`, omdat `Mat4` in de publieke header staat |
| 40 | Uniform vinden | op naam: `u_transform`. `CreateShader` zoekt de locatie één keer op en bewaart hem in `ShaderData`; ontbreekt de uniform, dan volgt een `LogWarn` en tekent de shader zonder transformatie |
| 41 | Draaiingen | drie losse functions: `Mat4 RotationX(float angle)`, `RotationY` en `RotationZ`. Geen `enum` voor de as (scheelt geen code) en geen willekeurige as (nog niet nodig; kan later ernaast komen, met deze drie als controle) |
| 42 | Hoek en richting | de hoek is in radialen, zoals `std::sin`, GLSL en glm; pi komt uit `<numbers>` (`std::numbers::pi_v<float>`). Een positieve hoek draait tegen de klok in, gezien vanaf de positieve kant van de as naar de oorsprong |
| 43 | Drie hoeken in één function | nu niet: `Rotation(const Vec3& angles)` moet zelf een volgorde kiezen en verstopt die (botst met besluit 36). Komt terug als dunne function bovenop de drie losse, zodra iets een stand als drie hoeken opslaat (een `Transform`-component in de Engine-module); dan leggen we één volgorde vast |
| 44 | `Vec4` (vervolg op 31) | toegevoegd, zo klein mogelijk: struct met de members `x`, `y`, `z`, `w` (elk `float`, default value `0.0f`) in `Vec4.h`. De aanroeper schrijft `w` altijd zelf: `1` is een punt, `0` een richting. `Mat4` blijft `float m[4][4]`. Aanleiding: een perspectiefmatrix controleren op wat hij met een punt doet, niet op zijn elementen |
| 45 | Matrix maal vector | `Vec4 operator*(const Mat4& mat, const Vec4& vec)` in `Mat4.h`/`Mat4.cpp`; `Mat4.h` include daarvoor `Vec4.h` |
| 46 | Perspectief | `Mat4 Perspective(float fovY, float aspect, float nearPlane, float farPlane)`. De camera staat in de oorsprong en kijkt langs de negatieve Z-as (right-handed); de diepte loopt van -1 tot 1 (standaard van OpenGL, geen `glClipControl`); `fovY` is de verticale kijkhoek in radialen. De namen `near` en `far` vermijden we: dat zijn macro's in de Windows-headers |
| 47 | Projectie naar de shader | Sandbox rekent `projection * model` uit en geeft die aan `DrawMesh`; de Renderer verandert niet. Aparte uniforms komen pas bij belichting |
| 48 | Tijd | voorlopig `std::chrono::steady_clock` rechtstreeks in Sandbox (testcode). Een klok in de engine ontwerpen we wanneer de game loop naar de Engine-module gaat; niet via `Window`/GLFW, want tijd is geen eigenschap van een venster |
| 49 | View-matrix | het omgekeerde van de plaatsing van de camera, opgebouwd met bestaande functions: de camera staat met `Translation(position) * RotationY(yaw)`, dus `view = RotationY(-yaw) * Translation(-position)`. Geen function in `Math` en geen algemene `Inverse`; een `Camera`-struct hoort later in de Engine-module. `LookAt(eye, target, up)` wordt een eigen stap zodra we om een voorwerp heen willen draaien (vraagt aftrekken, `Dot`, `Cross` en `Normalize` op `Vec3`) |
| 50 | Camera en model gescheiden naar de shader | ja, als eigen stap (14d): twee uniforms, `u_viewProjection` (per frame) en `u_model` (per voorwerp), met een function op de Renderer om de camera te zetten en `DrawMesh(mesh, shader, model)`. Aanleiding: meerdere voorwerpen en straks belichting, die de model-matrix los nodig heeft. Tot die stap rekent Sandbox `projection * view * model` uit (besluit 47) |
| 51 | Camera naar de shader (uitwerking van 50) | `void SetCamera(const Mat4& view, const Mat4& projection)`: de aanroeper geeft beide los, de Renderer rekent `projection * view` één keer uit. De camera blijft staan tot de volgende aanroep. `DrawMesh(mesh, shader, const Mat4& model)` stuurt alleen de model-matrix |
| 52 | Uniform buffer | de camera staat in een uniform buffer die alle shaders delen, niet in een gewone uniform per shader. In de shader: `layout(std140, binding = 0) uniform Camera { mat4 u_viewProjection; };`. Bindpunt 0, als `c_cameraBindingPoint` in `Renderer.cpp`. Het blok bevat alleen `u_viewProjection`; `view` en `projection` los komen erbij zodra een shader ze nodig heeft |
| 53 | Model-matrix | blijft een gewone uniform, nu met de naam `u_model` (was `u_transform`); opzoeken en waarschuwen zoals in besluit 40 |
| 54 | Kubus | in Sandbox als array, niet als function in de Renderer (regel 3: de Renderer kent geen vormen). Elk vlak een eigen effen kleur: 24 hoekpunten (vier per vlak) en 36 indices, want de kleur hoort bij het hoekpunt en elke hoek van de kubus hoort bij drie vlakken. Dezelfde opbouw is straks nodig voor normalen |
| 55 | Draairichting | alle driehoeken tegen de klok in, gezien vanaf de buitenkant (de standaard van OpenGL voor een voorkant). Face culling staat nog uit en wordt later een eigen kleine stap; met culling aan zou een dichte kubus ook zonder dieptebuffer goed lijken |
| 56 | Dieptebuffer | als eigen stap na de kubus (14f), zodat eerst zichtbaar is wat er zonder misgaat. De dieptetest staat altijd aan, één keer gezet in `Renderer::Create`; een function om hem uit te zetten komt pas bij doorzichtige voorwerpen of een interface over het beeld. `Clear(red, green, blue)` maakt kleur en diepte allebei leeg, de signatuur verandert niet. Het venster heeft al 24 bits diepte (standaard van GLFW) |
| 57 | Face culling | altijd aan, één keer gezet in `Renderer::Create` (`glEnable(GL_CULL_FACE)`), met de standaardinstellingen van OpenGL: tegen de klok in is de voorkant (`GL_CCW`), de achterkant valt weg (`GL_BACK`). Een function om het per voorwerp uit te zetten komt bij het eerste voorwerp dat van twee kanten zichtbaar moet zijn |
| 58 | Events uit het venster | een lijst, geen callback in de code van de aanroeper: `Window::PollEvents` maakt de lijst leeg en vult hem, `std::span<const WindowEvent> GetEvents() const` leest hem uit (geldig tot de volgende `PollEvents`). Binnen `Platform` komt GLFW bij het `Window`-object via `glfwSetWindowUserPointer` en een `static` member function als callback |
| 59 | Vorm van een event | een struct per soort met alleen zijn eigen gegevens, samen in een `std::variant`; uitlezen met `std::get_if`. Geen `enum` met een struct vol velden en geen basisklasse met `virtual`: een event heeft geen gedrag, zou per stuk op de heap moeten, en de soorten liggen vast |
| 60 | Naam en plek | `WindowEvent` in `Platform/WindowEvent.h`, niet `Event`: die naam blijft vrij. Events binnen het spel ("speel dit geluid", "vijand geraakt") worden later een eigen systeem in de Engine-module met eigen soorten; geen centraal type dat alle modules moeten kennen |
| 61 | Eigenaar van de lijst | het `Window`, zolang er één venster is. Bij een tweede venster verhuist de lijst naar een object in `Platform` boven de vensters, samen met het starten van GLFW en `PollEvents` (`glfwPollEvents` geldt voor alle vensters tegelijk) |
| 62 | Venstergrootte | het event is `FramebufferResized { width, height }`, in pixels (niet de venstergrootte in schermcoördinaten; die verschilt bij schaling). De grootte bij de start komt uit `FramebufferSize Window::GetFramebufferSize() const`. De Renderer krijgt `void SetViewport(int width, int height)`; Sandbox roept die aan en rekent de `projection` opnieuw uit, en slaat beide over bij grootte 0 (geminimaliseerd) |
| 63 | Operators op `Vec3` | losse functions, declaration in `Vec3.h` en definition in `Vec3.cpp` (niet `inline` in de header, zoals besluit 27). Voorlopig alleen `a + b` en unary `-a` (voor `Translation(-position)`); `a - b` komt als iets het nodig heeft |
| 64 | Toetsen | een eigen `enum class Key` in `Platform/Key.h`. Een toets is benoemd naar zijn plek op het toetsenbord, met de namen van de US-indeling, zoals GLFW. De eerste waarde is `Unknown` (de default value in de events), de laatste is `Count` (het aantal, voor de grootte van een array). Buiten `Platform` is geen GLFW-nummer zichtbaar. De lijst bevat alleen de toetsen die er nu toe doen; meer komt bij als iets het nodig heeft |
| 65 | Toets-events | `KeyPressed { key }` en `KeyReleased { key }` in de `WindowEvent`-variant (besluit 59). Een ingedrukt gehouden toets geeft geen herhaalde events (`GLFW_REPEAT` wordt overgeslagen). Toetsen die niet in `Key` staan, worden genegeerd |
| 66 | Ingedrukt houden | `bool Window::IsKeyDown(Key key) const`, uit een `std::array<bool, Count>` die de key-callback bijhoudt. Een event gebruik je voor iets dat één keer gebeurt (Escape sluit af), `IsKeyDown` voor iets dat duurt zolang de toets ingedrukt is (lopen). Een assert vangt `Count` af; `Unknown` geeft gewoon `false` |
| 67 | Vertalen van GLFW naar `Key` | `ToKey` in `Window.cpp` vertaalt letters, cijfers en F-toetsen met een offset per reeks, en de overige toetsen met een `switch`. Drie `static_assert`s controleren dat elke reeks in `Key` en in GLFW even lang is |
| 68 | Camera in Sandbox | een tijdelijke `Transform { position, rotationY }` in een naamloze namespace. De `view` wordt elk frame opnieuw uitgerekend uit positie en hoek (besluit 49), zonder matrices op te stapelen. WASD beweegt de camera langs zijn eigen assen; `RotateVec3Y` draait die beweging naar de assen van de wereld via `Mat4 * Vec4` met `w = 0`. Er komt niets bij in `Math`. Bij een tweede plek die richtingen draait (een camera in de Engine, normalen) kiezen we tussen `Vec3 TransformDirection(const Mat4&, const Vec3&)` en omzetten tussen `Vec3` en `Vec4` zoals in GLSL |
| 69 | Tijd per frame | Sandbox rekent met de tijd sinds het vorige frame (één `now()` per frame), niet meer met de tijd sinds de start (vervolg op 48) |
| 70 | Opzet van de ECS | naar het artikel van Austin Morlan (signature als `std::bitset`, een packed array per soort component met swap-with-last, systems met een eigen lijst entities), met aanpassingen: geen globale coordinator (de world gaat als `World&` mee), `std::unique_ptr` in plaats van `std::shared_ptr`, systems in een `std::vector` zodat de volgorde vastligt, geen `typeid(T).name()` als sleutel, onze namen en asserts. De ECS-code schrijft de gebruiker, net als rendering en `Math` |
| 71 | Module `ECS` | eigen statische library `EZSE::ECS`, hangt alleen van `Core` af; de loop en het render-system komen later in een module `Engine` daarboven |
| 72 | Systems | een klasse met `virtual void Update(World&, float deltaTime)`; de engine roept ze elk frame in volgorde aan. Een virtual call per system per frame kost niets meetbaars; components blijven kale structs zonder `virtual`. Members van een system zijn alleen gereedschap (zoals `Renderer&`), nooit toestand van het spel: die staat in components. Eerst alleen per frame met `deltaTime`; een vaste tick komt bij de eerste aanleiding (physics) |
| 73 | `Entity` | een struct met `std::uint32_t id`, `0` is geen entity, zoals de handles van de Renderer (een `using` geeft geen controle door de compiler). Ids worden niet hergebruikt; de id is de index plus één. Hergebruik met een generation komt zodra entities vaak worden aangemaakt en verwijderd |
| 74 | `EntityManager` | houdt alleen bij of een entity leeft, in een `std::vector<bool>` zonder maximum (met niet-hergebruikte ids zou `MAX_ENTITIES` een grens zijn op het aantal ooit). Geen aparte lijst met entities: de index is de entity. Signatures komen erbij met de components. `DestroyEntity` op een entity die niet leeft is een assert; `IsAlive` geeft dan gewoon `false` |
| 75 | `ECSTests` | eigen testprogramma zoals `MathTests` (besluit 34) |
| 76 | `ComponentArray<T>` | een sparse set, header-only (template). `std::vector<T>` met de components zonder gaten, `std::vector<Entity>` met per plek de eigenaar, en `std::vector<std::uint32_t> m_sparse` met per entity (index `id - 1`) de plek plus één, `0` = geen component. Geen `unordered_map` zoals Morlan: geen hash, en dezelfde afspraak als onze handles. Verwijderen met swap-with-last; de volgorde van de components ligt dus niet vast |
| 77 | Functions van `ComponentArray` | `Add(Entity, T component)` (by value, de vector in met `std::move`), `Remove`, `T& Get` en `bool Has` (zonder assert). `Add` assert op entity `0` en op een dubbel component; `Remove` en `Get` op een ontbrekend component. De basisklasse `IComponentArray` komt pas in 18c, met de `ComponentManager` |
| 78 | Omvang van 18c | alleen de `ComponentManager`. Signatures en het maximum aantal soorten (de grootte van de `std::bitset`) gaan naar 18d, waar de systems ze nodig hebben |
| 79 | Nummer per soort component | een eigen teller, geen `std::type_index` en geen `typeid(T).name()`: `inline std::uint32_t NextComponentType()` met een `static` teller en `template <typename T> std::uint32_t ComponentType()` met een `static const`, in `ComponentType.h`. Kleine opeenvolgende nummers (0, 1, 2 en zo verder) in volgorde van eerste gebruik, voor het hele programma. Direct bruikbaar als index en straks als bit; geen hash en geen RTTI. Werkt niet over DLL-grenzen, maar we hebben statische libraries (besluit 6) |
| 80 | `IComponentArray` | in `ComponentArray.h`: `virtual` destructor (`= default`) en `virtual void EntityDestroyed(Entity) = 0`. `ComponentArray<T>` erft ervan; zijn `EntityDestroyed` verwijdert het component als het er is, zonder assert. Dit is de enige virtual call in de opslag, en hij gebeurt niet per frame |
| 81 | `ComponentManager` | soorten met de hand registreren met `Register<T>()` (assert bij twee keer; `Get`, `Has` en dergelijke op een niet-geregistreerde soort is ook een assert). Opslag `std::vector<std::unique_ptr<IComponentArray>>` met als index `ComponentType<T>()`; plekken van soorten die in deze manager niet geregistreerd zijn blijven `nullptr`. Een private `GetArray<T>()` in een gewone en een `const`-versie zet de pointer met `static_cast` om naar `ComponentArray<T>`. `Add`, `Remove`, `Get` en `Has` (korte namen, zoals in `ComponentArray`) geven door; `EntityDestroyed` (in de `.cpp`) loopt langs alle arrays en slaat `nullptr` over. De manager controleert niet of een entity leeft: dat doet straks de `World` |
| 82 | Opsplitsing van stap 18 | 18d signatures, 18e `EntitySet`, 18f `System`, `MakeSignature` en `SystemManager`, 18g de `World`, 18h `Update`, 18i de module `Engine` met de loop en een klok, 18j Sandbox ombouwen (in 18e en 18g verder opgesplitst) |
| 83 | `Signature` | `using Signature = std::bitset<c_maxComponentTypes>;` met `inline constexpr std::uint32_t c_maxComponentTypes = 64;` in `Signature.h`. Een `using` en geen struct: een signature is een waarde met de bewerkingen van `std::bitset` (`set`, `reset`, `test`, `&`, `==`), geen handle, en er is niets om hem mee te verwisselen. 64 bits passen in één machinewoord. `Register` assert dat het nummer van een soort onder het maximum blijft |
| 84 | Eigenaar van de signatures | de `ComponentManager`, niet de `EntityManager` zoals bij Morlan. Een signature is een samenvatting van de components van een entity (elke bit is `Has<T>` voor één soort), dus de class die de components bewaart houdt hem bij: `std::vector<Signature> m_signatures` (index `id - 1`), bijgewerkt in `Add`, `Remove` en `EntityDestroyed`. Zo kunnen components en signature nooit uit elkaar lopen. De `World` (18f) blijft nodig om de managers te verbinden, maar hoeft de signature niet zelf bij te werken. De signature van een system komt in de `SystemManager` (18e) |
| 85 | Uitlezen | `Signature GetSignature(Entity) const`, by value (8 bytes; een kopie kan niet ongeldig worden), zonder assert: een lege signature voor entity `0`, voor een entity zonder components en voor een id voorbij `m_signatures` |
| 86 | `Update` van een system | komt pas in 18g, samen met de `World`: met alleen een forward declaration van `World` kan hij niet worden aangeroepen of getest |
| 87 | Entities per system | een eigen class `EntitySet` (`EntitySet.h`/`.cpp`): een sparse set zonder gegevens ernaast, met dezelfde techniek als `ComponentArray` (`m_entities` packed, `m_sparse` met plek plus één). Elk frame snel erover lopen, toevoegen, verwijderen en opzoeken in O(1). Geen `std::set` zoals Morlan (losse nodes, slecht voor de cache) en geen kale `std::vector` (opzoeken O(n)). Dat `ComponentArray` intern een `EntitySet` zou kunnen gebruiken, is een opschoonstap voor later. Op verzoek van de gebruiker door Claude geschreven, omdat de techniek al in 18b is geoefend |
| 88 | Gedrag van `EntitySet` | `Insert` doet niets als de entity er al in zit, `Remove` niets als hij er niet in zit (zoals `std::set`); de `SystemManager` hoeft dan niet eerst te controleren. Alleen `Insert` met entity `0` is een assert. Bewust anders dan `ComponentArray`, waar een dubbel component een fout is |
| 89 | Uitlezen van een `EntitySet` | `std::span<const Entity> GetEntities() const`, zoals `Window::GetEvents`; geldig tot de volgende `Insert` of `Remove` |
| 90 | `MakeSignature` | `template <typename... Ts> Signature MakeSignature()` in `Signature.h` (dat daarvoor `ComponentType.h` include): een fold expression `(signature.set(ComponentType<Ts>()), ...)`. Zonder types een lege signature. `std::bitset::set` gooit `std::out_of_range` bij een nummer van 64 of hoger; de assert in `Register` vangt dat eerder af |
| 91 | `System` | basisklasse in `System.h` (geen `.cpp`): `virtual ~System() = default` (systems worden via `std::unique_ptr<System>` verwijderd), een public `GetEntities()` (binnen de class gedefinieerd, dus vanzelf `inline`), en private `m_signature` en `m_entities` (een `EntitySet`). `friend class SystemManager;`: alleen de manager mag die veranderen, ook afgeleide systems niet |
| 92 | `SystemManager::Add` | `template <typename T, typename... Args> T& Add(Signature, Args&&... args)`: het system met `std::make_unique<T>(std::forward<Args>(args)...)` (perfect forwarding naar de constructor), signature zetten, de `T&` bewaren en dan met `std::move` achteraan in `std::vector<std::unique_ptr<System>> m_systems`. De volgorde van toevoegen wordt straks de volgorde van `Update`. De teruggegeven `T&` blijft geldig: het system zelf verhuist niet als de vector groeit. Twee systems van hetzelfde type mogen; niets zoekt op type. (Eerst stond hier een assert op een lege signature; vervangen door besluit 119) |
| 93 | Bijwerken van de systems | `EntitySignatureChanged(Entity, Signature)`: per system `Insert` als `(signature & system) == system` (de entity heeft minstens alle components van het system), anders `Remove`. `EntityDestroyed(Entity)`: `Remove` uit elk system. Beide in `SystemManager.cpp`. Een system dat wordt toegevoegd terwijl er al entities zijn, kent die niet; dat lost de `World` op (18g) |
| 94 | `World` | in `World.h`/`.cpp`: bezit `EntityManager`, `ComponentManager` en `SystemManager` by value (geen `unique_ptr` en geen `Init()` zoals Morlan; niets kan mislukken). Kan vanzelf niet gekopieerd worden. Geen getters voor de managers: de `World` is de enige ingang, zodat ze niet uit de pas kunnen lopen. De managers blijven publieke headers, omdat `World.h` ze by value bevat. De gebruiker schrijft de `World` zelf, omdat dit de code is die het meest gebruikt gaat worden |
| 95 | Functions van de `World` | `CreateEntity`, `DestroyEntity`, `IsAlive`, `RegisterComponent<T>`, `AddComponent<T>`, `RemoveComponent<T>`, `GetComponent<T>`, `HasComponent<T>` en `AddSystem<T>(signature, args...)`, mét "Component" in de naam (besluit 81). `DestroyEntity`, `AddComponent`, `RemoveComponent` en `GetComponent` assert dat de entity leeft; `HasComponent` niet (vragen is geen fout). `DestroyEntity` ruimt eerst components en systems op en verwijdert de entity als laatste. `AddComponent` en `RemoveComponent` geven daarna `GetSignature` door aan `EntitySignatureChanged` |
| 96 | Later toegevoegde systems | `AddSystem` loopt na het toevoegen langs alle uitgegeven ids (`EntityManager::GetCreatedCount()`, ook verwijderde) en geeft van elke levende entity de signature door aan `EntitySignatureChanged`. De andere systems krijgen dan ook een melding, wat onschuldig is (besluit 88). Daarmee is de beperking uit 18f opgelost |
| 97 | `System::Update` | `virtual void Update(World& world, float deltaTime) = 0;`: pure virtual, dus `System` is abstract en elk system moet hem schrijven (met `override`). `protected`: alleen afgeleide systems en de `SystemManager` (friend) komen erbij; van buitenaf roept niemand een system los aan. Een system krijgt de `World` als parameter (geen global zoals Morlan, besluit 70), omdat hij zelf alleen ids heeft en de components via de `World` ophaalt. `deltaTime` in seconden |
| 98 | `World` in `System.h` | een forward declaration `class World;` binnen `namespace ezse`: `World.h` include via `SystemManager.h` al `System.h`, en voor een `World&` is een incomplete type genoeg. Een concreet system include `World.h` in zijn eigen bestand |
| 99 | Systems aanroepen | `SystemManager::Update(World&, float)` loopt in volgorde van toevoegen over `m_systems`; `World::Update(float deltaTime)` geeft door met `*this`. Niet `const`: systems veranderen components |
| 100 | Module `Engine` | statische library `EZSE::Engine` (`EZSEEngine`), hangt `PUBLIC` af van Platform, Renderer en ECS (hun types staan in `Engine.h`) en `PRIVATE` van Core. Geen andere module hangt van `Engine` af, zodat Renderer en ECS los bruikbaar blijven. Aanleiding: venster, Renderer, loop, klok en viewport zou elk spel anders zelf schrijven |
| 101 | Hoe het spel code kwijt kan | de engine bezit de loop (zoals Unity, Godot en Bevy): het spel registreert components en voegt systems toe via `GetWorld()` en roept één keer `Run()` aan, dat pas terugkeert als het spel stopt. Geen basisklasse `Application` met virtual functions: al het gedrag zit in systems. Een function voor één frame (voor een editor of de browser) komt pas bij aanleiding |
| 102 | `Engine` | `static std::unique_ptr<Engine> Create(const WindowDesc&)` maakt venster en Renderer en logt wat mislukt; de constructor is `private` (dus `new Engine(...)` in `Create`, geen `make_unique`) en neemt beide `unique_ptr`'s over in de initializer list. Members in de volgorde `m_window`, `m_renderer`, `m_world`, `m_quitRequested`: opruimen gaat andersom, dus eerst de world met zijn systems (die een `Renderer&` kunnen hebben), dan de Renderer, als laatste het venster met de OpenGL-context. Getters `GetWorld`, `GetWindow` en `GetRenderer` geven referenties: systems krijgen `Window&` en `Renderer&` als gereedschap via hun constructor (besluit 72). Geen `EngineDesc` zolang er alleen venster-instellingen zijn |
| 103 | De loop | `Run`: viewport op de framebuffer-grootte, `lastFrame` (lokaal, `steady_clock::time_point`) vóór de loop; dan per frame `PollEvents`, `SetViewport` bij `FramebufferResized` (behalve bij grootte 0), `deltaTime` als `std::chrono::duration<float>` (seconden), `World::Update` en `SwapBuffers`. Tekenen (`Clear`, `SetCamera`, `DrawMesh`) en de projectie bij een andere venstergrootte zijn voor systems van het spel. De klok staat in `Run` zelf; een class `Clock` komt bij een tweede gebruiker (vervolg op besluit 48) |
| 104 | Stoppen | `Engine::Quit()` zet `m_quitRequested`; de loop maakt het lopende frame af en stopt, net als bij het sluiten van het venster. Een system dat mag stoppen (Escape) krijgt een `Engine&` via zijn constructor. `Window` krijgt geen function om zichzelf te sluiten |
| 105 | Opsplitsing van 18j | 18j de components, 18k `CameraSystem` en `RenderSystem` en Sandbox op de `Engine` met een stilstaande kubus, 18l de systems van het spel (draaien, camera besturen, Escape) |
| 106 | Plek van de components | in de module `Engine`, een header per component in `Components/` (submappen per soort, zie de conventies). Wat specifiek is voor het spel blijft in Sandbox. `EZSE/Engine/Components/Transform.h` naast `EZSE/Math/Transform.h`: de bouwstenen (`Translation`, `RotationY` en dergelijke) blijven in `Math`, de component en de functions die hem omzetten naar een matrix staan in `Engine`, omdat `Math` geen components kent |
| 107 | `Transform` | `Vec3 position`, `Vec3 rotation` (drie hoeken in radialen, Euler angles) en `Vec3 scale` (default `{1, 1, 1}`). Geen quaternion: dat komt bij vrije rotaties of animatie (gimbal lock). Vaste volgorde (vervolg op besluit 43): `ModelMatrix = Translation * RotationY * RotationX * RotationZ * Scale` (yaw, pitch, roll). `ViewMatrix(const Transform&)` is het omgekeerde zonder schaal: `RotationZ(-z) * RotationX(-x) * RotationY(-y) * Translation(-position)` (vervolg op besluit 49). Beide als losse functions in `Transform.cpp` |
| 108 | `MeshRenderer` | `MeshHandle mesh` en `ShaderHandle shader`; alleen handles (regel 4) |
| 109 | `Camera` | `fovY` (radialen, default 45 graden), `nearPlane` en `farPlane`. De plaatsing staat in de `Transform` van dezelfde entity; de `aspect` komt elk frame uit het venster (18k) |
| 110 | `EngineTests` | eigen testprogramma zoals `MathTests` (besluit 34), voor wat zonder venster te testen is; verwachte matrices met de hand opgebouwd of gecontroleerd op wat ze met een punt doen |
| 111 | Ingebouwde components | `Transform`, `MeshRenderer` en `Camera` komen altijd met de engine mee: de `Engine` registreert ze zelf in de body van zijn constructor (vermeld in `Engine.h`), zoals hij ook zelf venster en Renderer maakt. Het spel registreert alleen zijn eigen components. Nieuwe ingebouwde components worden op dezelfde plek geregistreerd |
| 112 | Twee systems voor tekenen | een system heeft één signature, en de camera (`Transform` + `Camera`) en wat getekend wordt (`Transform` + `MeshRenderer`) zijn verschillende entities. Eén system zou de camera als member moeten bewaren (toestand van het spel, besluit 72) of een query op de `World` nodig hebben die er niet is |
| 113 | `CameraSystem` | `CameraSystem(Window&, Renderer&)`, signature `Transform` + `Camera`. Gebruikt de eerste camera; zonder camera of bij een framebuffer van grootte 0 doet hij niets. De `aspect` komt elk frame uit `Window::GetFramebufferSize` (geen toestand nodig), daarna `SetCamera(ViewMatrix(transform), Perspective(...))` |
| 114 | `RenderSystem` | `RenderSystem(Renderer&, Vec3 clearColor)` (rood, groen en blauw in `x`, `y` en `z`), signature `Transform` + `MeshRenderer`. Eerst `Clear`, dan per entity `DrawMesh(mesh, shader, ModelMatrix(transform))`. De achtergrondkleur is een parameter van de constructor; in `Camera` pas bij meerdere camera's met een eigen achtergrond |
| 115 | Volgorde van de systems van de engine | het spel voegt `CameraSystem` en `RenderSystem` zelf toe, als laatste en in die volgorde, zodat ze na de systems van het spel lopen. De engine voegt ze niet zelf toe (dan zouden ze vooraan staan). Een aparte fase voor tekenen komt pas als deze afspraak knelt |
| 116 | Components ophalen | `GetComponent` geeft een `T&`; ophalen met `const T&` (of `const auto&`) om te lezen en `T&` om te veranderen. Met `auto` zonder `&` krijg je een kopie, en een wijziging daarop gaat stil verloren |
| 117 | Opsplitsing van 18l | 18l systems zonder entities en Escape, 18m `Vec3 * float` en de kubus draait, 18n `TransformDirection` en de camera met WASD en de pijltjes |
| 118 | Code van Sandbox | eigen systems en components van Sandbox staan in `main.cpp`, in een naamloze namespace (internal linkage: geen botsing met een gelijke naam in een andere `.cpp`, wat bij classes met virtual functions stil ongedefinieerd gedrag geeft). Verhuizen naar eigen bestanden als `main.cpp` te lang wordt |
| 119 | Systems zonder entities | een lege signature betekent dat het system geen entities krijgt: `SystemManager::EntitySignatureChanged` slaat systems met `m_signature.none()` over, en de assert in `Add` is weg. Zo'n system draait wel elk frame. Toevoegen met `MakeSignature<>()`. Geen nep-component om de regel te omzeilen |
| 120 | `GameControlSystem` | in Sandbox, `(Window&, Engine&)`, lege signature: bij `KeyPressed` met `Key::Escape` roept hij `Engine::Quit()` aan. Een system per verantwoordelijkheid, niet per toets: spel-brede toetsen (straks pauze, debug) komen hier ook bij. Toegevoegd vóór de systems van de engine |
| 121 | Gebeurtenissen in een ECS | invoer van het venster wordt gelezen met polling (`IsKeyDown`, `GetEvents`), zoals nu. Invoer vertalen naar acties (een input-system dat toetsen omzet naar betekenis) komt bij instelbare toetsen of een tweede invoerapparaat. Gebeurtenissen binnen het spel (besluit 60) worden waarschijnlijk event-queues per soort, zoals Bevy; event-components en callbacks zijn de alternatieven. Alles pas bij de eerste aanleiding |
| 122 | `Vec3 * float` | `Vec3 operator*(const Vec3& vec, float scalar)` in `Vec3.h`/`.cpp` (vervolg op besluit 63), met controles in `MathTests`. Nog geen `float * Vec3`, `+=` of `*=`: niemand gebruikt ze |
| 123 | Draaien in Sandbox | component `Spin { Vec3 speed; }` (radialen per seconde per as, zelfde volgorde als `Transform::rotation`) en `SpinSystem` (signature `Transform` + `Spin`): `rotation = rotation + speed * deltaTime`, met de `Transform` opgehaald als `Transform&`. Beide in de naamloze namespace van `main.cpp`; Sandbox registreert `Spin` zelf. De kubus draait met `{0.175, 0.25, 0}`, gelijk aan de oude Sandbox |
| 124 | Kijken met de muis in plaats van de pijltjes | eerst de muis in `Platform` (18n), daarna de camera met WASD en de muis (18o); de pijltjes komen niet terug. De muis geeft een afstand per frame en wordt dus met een gevoeligheid (radialen per pixel) vermenigvuldigd, niet met `deltaTime`. Kijken omhoog en omlaag (`rotation.x`) wordt begrensd op ongeveer ±89 graden |
| 125 | Muisbeweging | een toestand, geen event (zoals `IsKeyDown`): `MouseDelta Window::GetMouseDelta() const` met `struct MouseDelta { float x, y; }` in `Window.h` (geen `Vec2`: `Platform` hangt niet van `Math` af), in pixels, `x` naar rechts en `y` naar beneden. `PollEvents` vraagt na `glfwPollEvents` de positie op met `glfwGetCursorPos` en rekent het verschil met de vorige positie uit (in `double`, daarna `float`); geen callback. De constructor zet de beginpositie, zodat het eerste frame geen sprong geeft |
| 126 | Cursor vastzetten | `void Window::SetCursorCaptured(bool)`: `GLFW_CURSOR_DISABLED` (onzichtbaar, vast in het venster, virtuele positie zonder grens) of `GLFW_CURSOR_NORMAL`, en raw mouse motion als `glfwRawMouseMotionSupported()`. Zet daarna de vorige positie opnieuw, zodat het wisselen geen sprong geeft. Standaard uit. Muisknoppen, scrollen en de absolute positie komen pas als iets ze nodig heeft |
| 127 | `TransformDirection` | `Vec3 TransformDirection(const Mat4& mat, const Vec3& direction)` in `Math/Transform.h`/`.cpp` (vervolg op besluit 68): `mat * Vec4{x, y, z, 0}`, dus draaien en schalen zonder verplaatsing; met controles in `MathTests`. Geen omzetting tussen `Vec3` en `Vec4` zoals in GLSL: dan moet de aanroeper zelf weten dat `w = 0` een richting is |
| 128 | `CameraController` | component in Sandbox: `moveSpeed` (eenheden per seconde, default 1) en `lookSensitivity` (radialen per pixel, default 0,002); Sandbox registreert hem zelf |
| 129 | `CameraControlSystem` | in Sandbox, `explicit CameraControlSystem(Window&)`, signature `Transform` + `CameraController`. Kijken: `rotation.y -= delta.x * gevoeligheid` en `rotation.x -= delta.y * gevoeligheid` (besluit 42: positief om Y is naar links, positief om X is omhoog; `y` van de muis is naar beneden), zonder `deltaTime`, en `rotation.x` begrensd op ±89 graden met `std::clamp`. Lopen: WASD opgeteld in de eigen assen (W en S heffen elkaar op), gedraaid met `TransformDirection(RotationY(rotation.y), move)` zodat lopen horizontaal blijft, maal `moveSpeed * deltaTime`. Toegevoegd vóór het `CameraSystem`; Sandbox zet `SetCursorCaptured(true)` |
| 130 | Route na stap 18 | 19 `Length` en `Normalize` (en diagonaal lopen oplossen), 20 shaders uit bestanden (prettiger werken aan shaders, en de eerste foutafhandeling voor een ontbrekend bestand), 21 belichting (het vrije vertex-formaat met normalen, diffuse, daarna specular). Texturen volgen na belichting, omdat het vrije vertex-formaat er dan al is |
| 131 | `Length` en `Normalize` | losse functions in `Vec3.h`/`.cpp`, namen zoals in GLSL. Alleen deze twee, omdat het `CameraControlSystem` ze nu nodig heeft; `Dot`, `Cross` en `a - b` komen met `LookAt` of zodra C++-code ze gebruikt (belichting rekent `dot` in de shader) |
| 132 | Nulvector normaliseren | `Normalize` geeft de nulvector terug als de lengte kleiner is dan `1e-6f` (een lokale `const`), in plaats van `NaN` (zoals glm) of een assert (dan zou `Math` van `Core` afhangen). Aanleiding: de beweging van de camera is meestal nul. Zoals Unity met `normalized` |
| 133 | Plek van de shaders | `Sandbox/shaders/basic.vert` en `basic.frag` (de shaders van het spel); ze staan in `add_executable` zodat ze in Visual Studio te openen zijn. Shaders van de engine krijgen later een eigen map in `Engine` |
| 134 | Shaders vinden (tijdelijk, moet worden aangepast) | een vast pad, ingebakken door CMake: `target_compile_definitions(Sandbox PRIVATE EZSE_SANDBOX_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")`. Werkt hoe je ook start, en een aangepaste shader is meteen actief zonder bouwen; maar het pad bestaat alleen op deze computer. Een macro omdat de waarde van CMake komt. Moet vervangen worden door assets naast de executable met een assetsysteem in de engine (zie Daarna) |
| 135 | `ReadTextFile` | `std::optional<std::string> ReadTextFile(const std::filesystem::path&)` in `Core/File.h`/`.cpp`: een `std::ifstream` in binaire modus en de hele inhoud via `std::istreambuf_iterator`; kan het bestand niet geopend worden, dan `LogError` met het pad en `std::nullopt`. Een `std::optional` omdat een lege string niet te onderscheiden is van een leeg bestand. In `Core`, zodat ook de resource manager en het laden van modellen hem kunnen gebruiken |
| 136 | Fouten bij shaders in Sandbox | Sandbox leest de shaderbestanden vóór `Engine::Create` (geen venster bij een ontbrekend bestand) en stopt met `return 1` als een bestand niet gelezen kan worden of `CreateShader` een handle met id `0` geeft |
| 137 | Opdeling van stap 21 | 21a normalen in het vertex-formaat en de kubus, gecontroleerd met een normalen-shader; 21b een lichtrichting naar de shader, de normal matrix en diffuse belichting (Lambert); 21c ambient en specular (Blinn-Phong), waarvoor de camerapositie naar de shader moet |
| 138 | Normalen in `Vertex` | het vaste formaat wordt groter in plaats van vrij (besluit 24 blijft staan): `Vertex { position[3], color[3], normal[3] }`, attribuut 2 in `CreateMesh` (`glVertexArrayAttribFormat` met `offsetof(Vertex, normal)`, aanzetten en koppelen aan binding 0). Er is nog geen tweede formaat naast het eerste; het vrije formaat komt bij texturen. De kleur blijft, als eigen kleur van het oppervlak |
| 139 | Normalen van de kubus | per vlak één normaal, op alle vier de hoekpunten van dat vlak (kan dankzij de 24 hoekpunten van besluit 54): voor `+Z`, achter `-Z`, rechts `+X`, links `-X`, boven `+Y`, onder `-Y` |
| 140 | Normalen-shader | `Sandbox/shaders/normals.vert` en `.frag` tonen de normaal als kleur (`normal * 0.5 + 0.5`), om normalen te controleren; ze blijven staan als gereedschap om fouten te zoeken. Wisselen gaat door de bestandsnamen in `main.cpp` aan te passen. De normaal is nog die van de mesh, niet gedraaid naar de wereld |
| 141 | Normal matrix | in de vertex shader: `transpose(inverse(mat3(u_model))) * a_normal`, juist ook bij ongelijke schaal; per hoekpunt uitgerekend, wat voor een paar voorwerpen niets uitmaakt. In C++ uitrekenen (vraagt `Inverse` in `Math` en een extra uniform) komt als het uitmaakt. In de fragment shader opnieuw `normalize`, omdat interpoleren de normaal korter maakt |
| 142 | Licht naar de shader | een tweede uniform buffer op bindpunt 1 (`c_lightBindingPoint`), zoals de camera (besluit 52): `layout(std140, binding = 1) uniform Light { vec3 u_lightDirection; vec3 u_lightColor; };`. In `Renderer.cpp` een `LightData` met na elke `vec3` een `float` padding (in `std140` neemt een `vec3` 16 bytes in) en `static_assert(sizeof(LightData) == 32)`. `void Renderer::SetDirectionalLight(const Vec3& direction, const Vec3& color)` normaliseert de richting (van het licht naar het oppervlak) en vult de buffer; het licht blijft tot de volgende aanroep. De destructor ruimt de buffer op |
| 143 | Bron van het licht | Sandbox zet één richtingslicht vóór `Run()`: richting `(-0.5, -1, -0.7)`, wit. Een component `DirectionalLight` met een `LightSystem` (richting uit de `Transform`) komt als het licht moet bewegen of bij een eigen scene |
| 144 | Shader `lit` | `Sandbox/shaders/lit.vert` en `.frag` naast `basic` en `normals`: diffuse belichting (Lambert), `max(dot(n, -u_lightDirection), 0.0) * u_lightColor * color`, in wereldruimte. Zonder ambient zijn vlakken die van het licht af wijzen zwart (21c). Sandbox gebruikt `lit` |
| 145 | Ambient | een kleur in het blok `Light` (na richting en kleur, offset 32; `LightData` wordt 48 bytes), gezet met `Renderer::SetAmbientLight(const Vec3& color)`. Elke setter schrijft alleen zijn eigen deel van de buffer (`glNamedBufferSubData` met offset en grootte), zodat richtingslicht en ambient los te zetten zijn. Waarden worden eerst naar `float[3]` gekopieerd, niet als `Vec3` aan OpenGL gegeven (geen aanname over de indeling van `Vec3`). Sandbox: `(0.15, 0.15, 0.15)` |
| 146 | Camerapositie naar de shader | `SetCamera(const Mat4& view, const Mat4& projection, const Vec3& position)`; het blok `Camera` krijgt `vec3 u_cameraPosition` na `u_viewProjection` (`CameraData` van 80 bytes, met padding en `static_assert`). Het `CameraSystem` geeft `transform.position` mee. De buffer heet nu `m_cameraBuffer`. Shaders die alleen `u_viewProjection` declareren (`basic`, `normals`) werken ongewijzigd: een blok mag kleiner zijn dan de buffer |
| 147 | Specular | Blinn-Phong in wereldruimte in `lit.frag`: `halfway = normalize(-u_lightDirection + normalize(u_cameraPosition - worldPosition))`, `specular = pow(max(dot(n, halfway), 0), shininess) * specularStrength`, alleen maal de lichtkleur (een glans heeft de kleur van het licht). `lit.vert` geeft `worldPosition` door. Totaal: `color * (ambient + diffuse * lightColor) + specular * lightColor` |
| 148 | Glans van het oppervlak | `const float shininess = 128.0` en `specularStrength = 0.5` in `lit.frag`; alle belichte voorwerpen glanzen dus hetzelfde. Een materiaal (per voorwerp) wordt een eigen stap, waarschijnlijk samen met texturen |
| – | Shaderbron | als `std::string_view`, voorlopig als tekst in Sandbox |

## Huidige stand

Stappen 1 tot en met 12 zijn af en gecommit. Stap 13 (transformaties) is af, en van stap 14
zijn 14a tot en met 14f af; daarmee is stap 14 compleet. Stap 15 (face culling) en
stap 16 (events uit het venster, eerst alleen de grootte) zijn ook af: de module `Math` bevat de
struct `Mat4` met de eenheidsmatrix als beginwaarde en `operator*`, en `MathTests` controleert
alles met 62 controles (allemaal `[PASS]`). Verder zijn er `Vec3` met `a + b`, `-a`, `a * f`,
`Length` en `Normalize`, `Vec4` met `Mat4 * Vec4`,
`Translation`, `Scale`, de draaiingen `RotationX`, `RotationY` en `RotationZ`, `Perspective` en
`TransformDirection`.
De Renderer bewaart de camera in een uniform buffer (`SetCamera`) en `DrawMesh` geeft de
model-matrix door aan de uniform `u_model`. De dieptetest en face culling staan aan en `Clear`
maakt kleur en diepte leeg. Stap 17 (het toetsenbord) is ook af: `Window` geeft
`KeyPressed` en `KeyReleased` als events en heeft `IsKeyDown`.

Stap 18 (de ECS) is begonnen. Deel 18a: de module `ECS` met `Entity` en `EntityManager`.
Deel 18b: `ComponentArray<T>` als sparse set. Deel 18c: `ComponentType<T>()`, de basisklasse
`IComponentArray` en de `ComponentManager`. Deel 18d: signatures, bijgehouden door de
`ComponentManager`. Deel 18e: `EntitySet`. Deel 18f: `MakeSignature`, `System` en
`SystemManager`. Deel 18g: de `World`, die de managers verbindt. Deel 18h: `Update` op elk system en
`World::Update`. `ECSTests` controleert alles met 100 controles
(allemaal `[PASS]`, in Debug en Release). Deel 18i: de module `Engine` met de class `Engine`,
die venster, Renderer en `World` bezit en de loop draait. Deel 18j: de components `Transform`
(met `ModelMatrix` en `ViewMatrix`), `MeshRenderer` en `Camera`, gecontroleerd door
`EngineTests` (11 controles, allemaal `[PASS]`, in Debug en Release). Deel 18k: `CameraSystem` en
`RenderSystem`, en Sandbox draait nu op de `Engine`: een kubus-entity en een camera-entity op
`(0, 0, 3)`, met de twee systems. De kubus staat stil en wordt recht van voren getekend; het
venster van formaat veranderen werkt (door de gebruiker gezien). Draaien, WASD, de pijltjes en
Escape zijn tijdelijk weg en komen terug in 18l tot en met 18n. Deel 18l: een lege signature
betekent "geen entities" (systems zonder entities), en het `GameControlSystem` in Sandbox sluit
af met Escape (door de gebruiker gezien). `ECSTests` heeft nu 105 controles. Deel 18m:
`Vec3 * float`, en de kubus draait weer met een `Spin`-component en een `SpinSystem` in
Sandbox (door de gebruiker gezien). Deel 18n: `Window` geeft de muisbeweging per frame
(`GetMouseDelta`) en kan de cursor vastzetten (`SetCursorCaptured`). Deel 18o: `TransformDirection`
in `Math`, en de camera in Sandbox loopt met WASD en kijkt rond met de muis (door de gebruiker
gezien). Daarmee is stap 18 af: Sandbox doet weer alles wat hij voor de ECS deed (draaiende
kubus, lopen, rondkijken, Escape), nu volledig met entities en systems.

Stap 19 is ook af: `Length` en `Normalize` op `Vec3`, en het `CameraControlSystem` normaliseert de
beweging, zodat diagonaal lopen even snel gaat als rechtdoor. Stap 20 ook: de shaders staan in
`Sandbox/shaders/` en worden met `ReadTextFile` (in `Core`) ingelezen; een ontbrekend bestand of
een fout in de shader stopt Sandbox netjes (door de gebruiker gezien). Stap 21 (belichting) is
begonnen met 21a: `Vertex` heeft een normaal op attribuut 2, de kubus heeft per vlak een normaal,
en de normalen-shader toont ze als kleur (door de gebruiker gezien). Deel 21b: de Renderer
heeft een richtingslicht in een tweede uniform buffer (`SetDirectionalLight`), en Sandbox tekent
met de shader `lit` (diffuse belichting). Deel 21c: ambient en specular (Blinn-Phong), met de
camerapositie in het blok `Camera` (door de gebruiker gezien). Daarmee is stap 21 af.

Publieke interface van `Renderer`:

```cpp
static std::unique_ptr<Renderer> Create(GLLoadFunction loadFunction);
MeshHandle   CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices);
void         DestroyMesh(MeshHandle mesh);
ShaderHandle CreateShader(std::string_view vertexSource, std::string_view fragmentSource);
void         DestroyShader(ShaderHandle shader);
void         DrawMesh(MeshHandle mesh, ShaderHandle shader, const Mat4& model);
void         SetCamera(const Mat4& view, const Mat4& projection, const Vec3& position);
void         SetDirectionalLight(const Vec3& direction, const Vec3& color);
void         SetAmbientLight(const Vec3& color);
void         SetViewport(int width, int height);
void         Clear(float red, float green, float blue);
```

Publieke interface van `Window`, voor zover nieuw in stap 16, 17 en 18n:

```cpp
void                         PollEvents();
std::span<const WindowEvent> GetEvents() const;
FramebufferSize              GetFramebufferSize() const;
bool                         IsKeyDown(Key key) const;
MouseDelta                   GetMouseDelta() const;
void                         SetCursorCaptured(bool captured);
```

Bekende beperkingen, bewust zo gelaten:

- Tijdens het slepen aan de rand van het venster staat het beeld stil: op Windows blokkeert
  `glfwPollEvents` zolang de muisknop is ingedrukt. Daarna staan alle tussenliggende groottes
  achter elkaar in de lijst.
- Eén venster tegelijk: GLFW wordt gestart in `Window::Create` en afgesloten in de destructor.
- De attribuutnummers in `CreateMesh` en de `location`-nummers in de shader zijn een
  afspraak die nergens wordt afgedwongen.
- Hetzelfde geldt voor de naam `u_model` en voor het blok `Camera` op bindpunt 0: de Renderer
  en de shader moeten het allebei zo doen. Een shader zonder het blok geeft geen waarschuwing.
- De uniform buffers van de camera en het licht krijgen geen begininhoud: wie tekent zonder eerst
  `SetCamera` en (met `lit`) `SetDirectionalLight` aan te roepen, tekent met onbepaalde waarden.
- Er is precies één richtingslicht; meer lichten of een puntlicht vragen een ander blok in de
  shader.
- Alle belichte voorwerpen hebben hetzelfde materiaal: `shininess` en `specularStrength` zijn
  constanten in `lit.frag` (besluit 148).
- De normal matrix wordt per hoekpunt in de shader uitgerekend (besluit 141).
- Face culling staat altijd aan: een los vlak is van achteren gezien onzichtbaar, en een
  driehoek in de verkeerde draairichting verdwijnt (besluit 57).
- `EZSE_DEBUG_BREAK` gebruikt `__debugbreak()` en werkt dus alleen met MSVC.
- **Moet worden aangepast:** het pad naar de shaders van Sandbox is door CMake ingebakken als
  absoluut pad (`EZSE_SANDBOX_SHADER_DIR`, besluit 134) en bestaat alleen op deze computer. Een
  gebouwde Sandbox werkt dus niet op een andere computer of na het verplaatsen van de map.
- `Key` bevat geen numeriek toetsenblok, en de modifiers (Shift, Ctrl) gaan niet met het
  event mee.
- De `static_assert`s bij `ToKey` merken een tussengevoegde toets in een reeks op, maar niet
  een andere volgorde binnen een reeks.
- Entity-ids worden niet hergebruikt: de `std::vector<bool>` in `EntityManager` groeit met elke
  entity die ooit is aangemaakt (besluit 73). Na 2³² entities loopt de id over naar `0`; daar is
  geen controle voor.
- `DestroyEntity` op een entity die niet leeft is in Release (zonder assert) ongedefinieerd
  gedrag.
- `m_sparse` in elke `ComponentArray` groeit tot de hoogste id die het component kreeg, ook
  voor entities zonder dat component (4 bytes per entity per soort component).
- Een `T&` uit `ComponentArray::Get` wordt ongeldig bij een volgende `Add` (de vector kan
  groeien) of `Remove` (swap-with-last); niet bewaren over die aanroepen heen.
- Het nummer van een soort component hangt af van de volgorde van eerste gebruik, en geldt
  voor het hele programma. Het is dus niet geschikt om op te slaan (bijvoorbeeld in een
  bestand) en kan per run verschillen.
- Elk nummer telt mee voor het maximum van 64, ook van een soort die nooit wordt
  geregistreerd (`ComponentType<T>()` deelt het nummer uit bij het eerste gebruik).
- `World::AddSystem` loopt langs elke id die ooit is uitgegeven, ook van verwijderde entities;
  dat kost tijd naar het aantal entities ooit, maar alleen bij het toevoegen van een system.
- Wie de managers zelf gebruikt in plaats van de `World`, moet ze zelf in de pas houden (de
  tests doen dat bewust).
- Een system mag in zijn `Update` geen components toevoegen of verwijderen (of entities
  verwijderen) waardoor een entity in of uit zijn eigen `EntitySet` gaat: de span uit
  `GetEntities()` wordt dan ongeldig terwijl hij erover loopt. Componentwaarden veranderen mag
  wel. Een command buffer (wijzigingen na de loop uitvoeren) komt bij de eerste aanleiding.
- `Engine::Run` zet de viewport bij de start zonder te controleren op grootte 0 (de loop doet
  dat wel).
- De `Engine`, het `CameraSystem` en het `RenderSystem` openen een venster en kunnen dus niet
  in een testprogramma; ze worden alleen gecontroleerd door Sandbox te starten.
- Rotaties als drie hoeken hebben gimbal lock: bij een kanteling van een kwartslag om X vallen
  de draaiingen om Y en Z samen. Voor de camera en de kubus van nu speelt dat niet.
- Het `CameraSystem` gebruikt de eerste camera; bij meer camera's is "de eerste" willekeurig
  (de volgorde in een `EntitySet` ligt niet vast). Een veld `active` komt bij de tweede camera.
- Dat het `CameraSystem` vóór het `RenderSystem` en beide na de systems van het spel worden
  toegevoegd, is een afspraak die nergens wordt afgedwongen (besluit 115).
- De hoeken in `Transform::rotation` worden bij het draaien steeds groter; na lange tijd wordt
  een `float` van duizenden radialen onnauwkeurig en gaat het draaien schokken. Terugbrengen
  tot 0 tot 2π (`std::fmod`) komt als het ergens speelt.

## Volgende stap: nog te kiezen

De route van besluit 130 (19 `Length` en `Normalize`, 20 shaders uit bestanden, 21 belichting) is
af. Leg de gebruiker de lijst onder **Daarna** voor. Volgens besluit 130 liggen texturen voor de
hand (het vrije vertex-formaat, besluit 24, met UV's; plaatjes inladen; samen met materialen,
besluit 148). Stap 21 ging in deze delen: 21a normalen (besluiten 137 tot en met 140), 21b diffuse
(141 tot en met 144), 21c ambient en specular (145 tot en met 148).

Stap 18 (de ECS, de module `Engine` en Sandbox daarop) ging in deze delen:

| Deel | Onderwerp | Stand |
|---|---|---|
| 18a | module `ECS`, `Entity` en `EntityManager`, `ECSTests` (besluiten 70 tot en met 75) | af |
| 18b | `ComponentArray<T>`: packed array per soort component, met swap-with-last (besluiten 76 en 77) | af |
| 18c | `ComponentManager`: soorten components registreren (besluiten 78 tot en met 81) | af |
| 18d | signatures, bijgehouden door de `ComponentManager` (besluiten 82 tot en met 85) | af |
| 18e | `EntitySet`: de entities van een system (besluiten 86 tot en met 89) | af |
| 18f | `System`, `MakeSignature<...>()` en `SystemManager` (besluiten 90 tot en met 93) | af |
| 18g | de `World`: de managers verbinden (besluiten 94 tot en met 96) | af |
| 18h | `virtual Update(World&, float)` op `System` en `World::Update` (besluiten 97 tot en met 99) | af |
| 18i | de module `Engine` met de game loop en een klok (besluiten 100 tot en met 104) | af |
| 18j | de components `Transform`, `MeshRenderer` en `Camera` (besluiten 105 tot en met 110) | af |
| 18k | `CameraSystem` en `RenderSystem` in `Engine/Systems/`, Sandbox op de `Engine` met een stilstaande kubus (besluiten 111 tot en met 116) | af |
| 18l | systems zonder entities (lege signature) en Escape in een `GameControlSystem` (besluiten 117 tot en met 121) | af |
| 18m | `Vec3 * float` in `Math`, `Spin` en `SpinSystem` in Sandbox (besluiten 122 en 123) | af |
| 18n | de muis in `Platform`: `GetMouseDelta` en `SetCursorCaptured` (besluiten 124 tot en met 126) | af |
| 18o | `TransformDirection` in `Math`, `CameraController` en `CameraControlSystem` in Sandbox (besluiten 127 tot en met 129) | af |

Stap 14 (de draaiende 3D-kubus, besluiten 26 tot en met 62) ging in deze delen:

| Deel | Onderwerp | Stand |
|---|---|---|
| 14a | `Vec4` en `Mat4 * Vec4` | af |
| 14b | `Perspective`, met controles in `MathTests`, en zichtbaar in Sandbox | af |
| 14c | de camera: de view-matrix, in Sandbox | af |
| 14d | de camera naar de Renderer, met een uniform buffer (besluiten 50 tot en met 53) | af |
| 14e | de kubus in Sandbox, nog zonder dieptebuffer (besluiten 54 en 55) | af |
| 14f | de dieptebuffer (besluit 56) | af |

Achtergrond die al is uitgelegd: `m[kolom][rij]`, dus de eerste index kiest een kolom van
vier floats. In een initializer staat elke kolom op een eigen regel, waardoor de matrix
gekanteld staat ten opzichte van de schrijfwijze op papier. Voor de eenheidsmatrix maakt dat
niets uit; voor een verplaatsing wel (`tx`, `ty`, `tz` staan in `m[3]`).

Achtergrond bij de draaiingen: de kolom van `M` kiest de invoer (x, y of z), de rij kiest de
uitkomst. `RotationX` en `RotationY` zijn zonder perspectief niet op het scherm te beoordelen
(ze drukken het vierkant alleen plat); de richting is vastgelegd met de kwartslag-controles in
`MathTests`.

Achtergrond bij `Perspective`: de matrix zet de afstand tot de camera in `w` (`w' = -z`) en de
videokaart deelt daarna door `w`; daarom moet `m[3][3]` van de eenheidsmatrix nul worden. De
tests doen die deling zelf met de helper `PerspectiveDivide`.

Achtergrond bij de uniform buffer: de id van de buffer geeft OpenGL (`glCreateBuffers`), het
nummer van het bindpunt kiezen we zelf. `glBindBufferBase` legt de buffer in dat genummerde
vakje en de shader leest eruit met `binding = 0`. `std140` legt de indeling in het geheugen
vast; een `Mat4` van ons past daar precies op een `mat4`.

## Daarna

Op de lijst, zonder vaste volgorde: `LookAt` met `Dot`, `Cross` en `a - b` op `Vec3` (besluit 49),
texturen (met het vrije vertex-formaat) en materialen (besluit 148), meer lichten of een
puntlicht, het licht als entity met een `LightSystem` (besluit 143), de normal matrix in C++
(vraagt `Inverse`), muisknoppen en scrollen
als `WindowEvent`, shaders en modellen uit bestanden laden,
een resource manager in de Engine-module, gebeurtenissen binnen het spel (besluit 121), en pauze
en menu's: de loop blijft in `Engine::Run`; pauze wordt een toestand van het spel met systems per
toestand (de gameplay-systems slaan hun `Update` over, tekenen gaat door), een hoofdmenu een eigen
`World` (scene) die de `Engine` als actieve wereld kan wisselen; beide vragen een interface over
het beeld (tekst, knoppen) in de Renderer. **Assets naast de executable** (vervangt het
ingebakken pad van besluit 134): CMake kopieert shaders en later andere bestanden naast de
executable, en de engine vindt ze ten opzichte van zijn eigen map, als begin van een
assetsysteem; eventueel shaders opnieuw laden terwijl het programma draait.
