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
```

`MathTests` en `ECSTests` openen geen venster, printen per controle `[PASS]` of `[FAIL]` en
eindigen met exit code `0` als alles klopt. Claude draait ze zelf na elke wijziging in `Math`
of `ECS`.

Release: `cmake --build build --config Release`. De map `build/` staat niet in git.
In Visual Studio: open `build/EZSE.sln`. Bij een nieuw gegenereerde `build/` is Sandbox al
het startproject (`VS_STARTUP_PROJECT`). In een map die al een `.vs/` heeft stel je dat
eenmalig met de hand in (rechtsklik op Sandbox, *Set as Startup Project*); anders geeft F5
"Unable to start program ... ALL_BUILD, toegang geweigerd".

## Architectuur

Aparte statische libraries met afhankelijkheden in één richting. De build dwingt de grenzen af.

```
              Sandbox
             /   |    \
      Platform   |    Renderer ───► Math ◄── MathTests
        |   \    |    /    |
      GLFW    Core       glad
               ▲
              ECS ◄── ECSTests

      (Sandbox linkt ook Math, voor `Translation`; Sandbox gebruikt ECS nog niet)
```

| Module | Bevat | Hangt af van |
|---|---|---|
| `Core` | logging (`LogInfo` enz.), `EZSE_ASSERT` | niets |
| `Math` | `Mat4` met `operator*` (maal `Mat4` en maal `Vec4`), `Vec3` met `operator+` en unary `operator-`, `Vec4`; `Translation`, `Scale`, `RotationX`, `RotationY`, `RotationZ` en `Perspective` (in `Transform.h`); testprogramma `MathTests` in `Math/tests/` | niets |
| `ECS` | `World` (de ingang: `CreateEntity`, `DestroyEntity`, `IsAlive`, `RegisterComponent`, `AddComponent`, `RemoveComponent`, `GetComponent`, `HasComponent`, `AddSystem`); `Entity` en `EntityManager` (`CreateEntity`, `DestroyEntity`, `IsAlive`, `GetCreatedCount`); `ComponentArray<T>` (`Add`, `Remove`, `Get`, `Has`) met basisklasse `IComponentArray`; `ComponentType<T>()`; `ComponentManager` (`Register`, `Add`, `Remove`, `Get`, `Has`, `EntityDestroyed`, `GetSignature`); `Signature` en `c_maxComponentTypes`; `EntitySet` (`Insert`, `Remove`, `Contains`, `GetEntities`); `MakeSignature<Ts...>()`; basisklasse `System` en `SystemManager` (`Add`, `EntitySignatureChanged`, `EntityDestroyed`); testprogramma `ECSTests` in `ECS/tests/` | Core (PUBLIC, want `ComponentArray.h` gebruikt `EZSE_ASSERT`) |
| `Platform` | `Window` (GLFW, verborgen achter eigen interface) met `IsKeyDown`, `WindowEvent` (`FramebufferResized`, `KeyPressed`, `KeyReleased`) en `Key` | Core, GLFW (PRIVATE) |
| `Renderer` | OpenGL laden, meshes, shaders, tekenen | Math (PUBLIC), Core, glad (PRIVATE) |
| `Sandbox` | testprogramma dat de modules verbindt | alle vier |
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
| 24 | Hoekpunten | één vast `Vertex`-formaat (positie op attribuut 0, kleur op 1); een vrij formaat ontwerpen we bij het eerste tweede formaat, waarschijnlijk texturen |
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
| 92 | `SystemManager::Add` | `template <typename T, typename... Args> T& Add(Signature, Args&&... args)`: assert op een lege signature (die zou elke entity krijgen), het system met `std::make_unique<T>(std::forward<Args>(args)...)` (perfect forwarding naar de constructor), signature zetten, de `T&` bewaren en dan met `std::move` achteraan in `std::vector<std::unique_ptr<System>> m_systems`. De volgorde van toevoegen wordt straks de volgorde van `Update`. De teruggegeven `T&` blijft geldig: het system zelf verhuist niet als de vector groeit. Twee systems van hetzelfde type mogen; niets zoekt op type |
| 93 | Bijwerken van de systems | `EntitySignatureChanged(Entity, Signature)`: per system `Insert` als `(signature & system) == system` (de entity heeft minstens alle components van het system), anders `Remove`. `EntityDestroyed(Entity)`: `Remove` uit elk system. Beide in `SystemManager.cpp`. Een system dat wordt toegevoegd terwijl er al entities zijn, kent die niet; dat lost de `World` op (18g) |
| 94 | `World` | in `World.h`/`.cpp`: bezit `EntityManager`, `ComponentManager` en `SystemManager` by value (geen `unique_ptr` en geen `Init()` zoals Morlan; niets kan mislukken). Kan vanzelf niet gekopieerd worden. Geen getters voor de managers: de `World` is de enige ingang, zodat ze niet uit de pas kunnen lopen. De managers blijven publieke headers, omdat `World.h` ze by value bevat. De gebruiker schrijft de `World` zelf, omdat dit de code is die het meest gebruikt gaat worden |
| 95 | Functions van de `World` | `CreateEntity`, `DestroyEntity`, `IsAlive`, `RegisterComponent<T>`, `AddComponent<T>`, `RemoveComponent<T>`, `GetComponent<T>`, `HasComponent<T>` en `AddSystem<T>(signature, args...)`, mét "Component" in de naam (besluit 81). `DestroyEntity`, `AddComponent`, `RemoveComponent` en `GetComponent` assert dat de entity leeft; `HasComponent` niet (vragen is geen fout). `DestroyEntity` ruimt eerst components en systems op en verwijdert de entity als laatste. `AddComponent` en `RemoveComponent` geven daarna `GetSignature` door aan `EntitySignatureChanged` |
| 96 | Later toegevoegde systems | `AddSystem` loopt na het toevoegen langs alle uitgegeven ids (`EntityManager::GetCreatedCount()`, ook verwijderde) en geeft van elke levende entity de signature door aan `EntitySignatureChanged`. De andere systems krijgen dan ook een melding, wat onschuldig is (besluit 88). Daarmee is de beperking uit 18f opgelost |
| – | Shaderbron | als `std::string_view`, voorlopig als tekst in Sandbox |

## Huidige stand

Stappen 1 tot en met 12 zijn af en gecommit. Stap 13 (transformaties) is af, en van stap 14
zijn 14a tot en met 14f af; daarmee is stap 14 compleet. Stap 15 (face culling) en
stap 16 (events uit het venster, eerst alleen de grootte) zijn ook af: de module `Math` bevat de
struct `Mat4` met de eenheidsmatrix als beginwaarde en `operator*`, en `MathTests` controleert
alles met 44 controles (allemaal `[PASS]`). Verder zijn er `Vec3` met `a + b` en `-a`, `Vec4` met `Mat4 * Vec4`,
`Translation`, `Scale`, de draaiingen `RotationX`, `RotationY` en `RotationZ`, en `Perspective`.
De Renderer bewaart de camera in een uniform buffer (`SetCamera`) en `DrawMesh` geeft de
model-matrix door aan de uniform `u_model`. De dieptetest en face culling staan aan en `Clear`
maakt kleur en diepte leeg. Sandbox tekent een kubus met zes effen gekleurde vlakken die om
twee assen draait. Het tekengebied en de `aspect` volgen de grootte van
het venster via `FramebufferResized`. Stap 17 (het toetsenbord) is ook af: `Window` geeft
`KeyPressed` en `KeyReleased` als events en heeft `IsKeyDown`. In Sandbox begint de camera op
`(0, 0, 3)`; je loopt met WASD, draait met de pijltjes links en rechts, en Escape sluit af.

Stap 18 (de ECS) is begonnen. Deel 18a: de module `ECS` met `Entity` en `EntityManager`.
Deel 18b: `ComponentArray<T>` als sparse set. Deel 18c: `ComponentType<T>()`, de basisklasse
`IComponentArray` en de `ComponentManager`. Deel 18d: signatures, bijgehouden door de
`ComponentManager`. Deel 18e: `EntitySet`. Deel 18f: `MakeSignature`, `System` en
`SystemManager`. Deel 18g: de `World`, die de managers verbindt. `ECSTests` controleert alles
met 96 controles
(allemaal `[PASS]`, in Debug en Release). Sandbox gebruikt de ECS nog niet.

Publieke interface van `Renderer`:

```cpp
static std::unique_ptr<Renderer> Create(GLLoadFunction loadFunction);
MeshHandle   CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices);
void         DestroyMesh(MeshHandle mesh);
ShaderHandle CreateShader(std::string_view vertexSource, std::string_view fragmentSource);
void         DestroyShader(ShaderHandle shader);
void         DrawMesh(MeshHandle mesh, ShaderHandle shader, const Mat4& model);
void         SetCamera(const Mat4& view, const Mat4& projection);
void         SetViewport(int width, int height);
void         Clear(float red, float green, float blue);
```

Publieke interface van `Window`, voor zover nieuw in stap 16 en 17:

```cpp
void                         PollEvents();
std::span<const WindowEvent> GetEvents() const;
FramebufferSize              GetFramebufferSize() const;
bool                         IsKeyDown(Key key) const;
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
- De uniform buffer van de camera krijgt geen begininhoud: wie tekent zonder eerst `SetCamera`
  aan te roepen, tekent met onbepaalde waarden.
- Face culling staat altijd aan: een los vlak is van achteren gezien onzichtbaar, en een
  driehoek in de verkeerde draairichting verdwijnt (besluit 57).
- `EZSE_DEBUG_BREAK` gebruikt `__debugbreak()` en werkt dus alleen met MSVC.
- Sandbox controleert de handle van `CreateShader` niet (testcode).
- Diagonaal lopen in Sandbox gaat ongeveer 1,4 keer zo snel als rechtdoor (testcode).
- `Key` bevat geen numeriek toetsenblok, en de modifiers (Shift, Ctrl) gaan niet met het
  event mee.
- De `static_assert`s bij `ToKey` merken een tussengevoegde toets in een reeks op, maar niet
  een andere volgorde binnen een reeks.
- De naam `Transform` in Sandbox kan later botsen met een `Transform`-component in de
  Engine-module.
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

## Volgende stap: 18h, `Update`

Stap 18 bouwt de ECS op in de volgorde van het artikel van Morlan (besluit 70). Per deel eerst
de keuzes bespreken; de punten uit zijn code die we anders doen staan in besluit 70.

| Deel | Onderwerp | Stand |
|---|---|---|
| 18a | module `ECS`, `Entity` en `EntityManager`, `ECSTests` (besluiten 70 tot en met 75) | af |
| 18b | `ComponentArray<T>`: packed array per soort component, met swap-with-last (besluiten 76 en 77) | af |
| 18c | `ComponentManager`: soorten components registreren (besluiten 78 tot en met 81) | af |
| 18d | signatures, bijgehouden door de `ComponentManager` (besluiten 82 tot en met 85) | af |
| 18e | `EntitySet`: de entities van een system (besluiten 86 tot en met 89) | af |
| 18f | `System`, `MakeSignature<...>()` en `SystemManager` (besluiten 90 tot en met 93) | af |
| 18g | de `World`: de managers verbinden (besluiten 94 tot en met 96) | af |
| 18h | `virtual Update(World&, float)` op `System` en `World::Update` | volgende |
| 18i | de module `Engine` met de game loop en een klok | |
| 18j | Sandbox ombouwen naar entities en systems | |

Open punten voor 18h: `virtual void Update(World&, float deltaTime)` op `System` (besluit 72),
pure virtual (`= 0`) of met een lege body; `World.h` include `System.h`, terwijl `System.h`
`World` nodig heeft (forward declaration `class World;`; een referentie heeft geen volledige
definitie nodig); wie de systems in volgorde aanroept (`World::Update` die doorgeeft aan een
function op de `SystemManager`, of de `World` zelf); `const` of niet.

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

Op de lijst, zonder vaste volgorde: `LookAt` met de bewerkingen op `Vec3` (besluit 49), belichting (vraagt normalen en de
model-matrix los, besluit 50), texturen (met het vrije vertex-formaat), de
muis als nieuwe soort `WindowEvent` (daarna de camera ook met de muis besturen), shaders en modellen uit bestanden laden,
en de Engine-module met de game loop en een resource manager (de ECS is stap 18).
