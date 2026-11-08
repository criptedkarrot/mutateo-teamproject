# Mutateo gameplay loop

> Spawn → guide welcomes you → glowing starter carrot → plant it → fetch scrap / rope / plastic while it
> grows → craft the water gun → crops ready → hit the harvest switch → crops become mutants → kill them →
> grab the drops → deposit for coins → upgrade station → replant → loop forever (upgrades make it incremental)

Everything is already set up in `Lvl_FirstPerson`. Press Play to try it.

## What's in the level (Outliner → folder "Mutateo")

| Actor | What it does |
|---|---|
| `Mutateo_LoopDirector` | Runs the loop: voice lines (subtitles), objective text, coin counter, highlights where to go next, water gun firing |
| `Mutateo_CraftingStation` | Workbench. Takes 1 scrap, 1 rope and 1 plastic, then spawns `BP_Pickup_WaterGun` |
| `Mutateo_HarvestSwitch` | Gold box beside the farm. Turns every grown crop into a `BP_Mutant` and clears the plot |
| `Mutateo_DepositStation` | Replaces the old static deposit-station mesh (same spot). Buys carrots for 5 coins each and leaves 1 for replanting |
| `Mutateo_UpgradeStation` | Gold pillar. Opens the upgrade menu |
| `BP_Plantplot_2/3/4` | Three extra plots next to your original one |

New assets in `Content/Mutateo`: `BP_Mutant`, `BP_Pickup_Carrot`, `BP_Pickup_WaterGun`, `M_MutateoHighlight`.

**Interact key is F** (the `IA_Interact` mapping in `IMC_EGUI_DemoInputs`). Left mouse fires the water gun once it's crafted *and equipped*.

## No Blueprint wiring needed
The C++ reads your existing Blueprints directly:
- **Inventory:** the `Inventoryitems` array on `inventorymanagercomponant` (count and iteminstance). After it removes items it calls `Oninventorychanged`, so your UI refreshes.
- **Crops:** any actor with an `Isready` bool (`bp_carrotplant`) within about 1.2 m of a plot counts as that plot's crop.

If you'd rather control this yourself, add the **Mutateo Inventory** interface to the inventory component, or the **Mutateo Crop Plot** interface to `BP_Plantplot`. Whichever you add takes priority over the automatic reading.

## Tuning (select the actor → Details)
- **Voice lines:** Director → Mutateo|Voice. Edit the text or add audio clips. "First loop" lines play once; "Repeat" lines play on every later loop.
- **Upgrades:** UpgradeStation → Upgrades (cost, growth, max level, value per level). Add new entries freely, and read them anywhere with **Get Upgrade Value**.
- **Economy:** DepositStation → Sellable Items. HarvestSwitch → Mutant Class. `BP_Mutant` → health, damage, speed, Drop Class.
- **Water gun:** Director → Mutateo|Combat (damage, range, fire rate, splash effects).
- **Growth Speed upgrade:** in `bp_carrotplant`, divide the grow time by **Get Upgrade Value (GrowthSpeed, 1)**. That's the only upgrade that needs a Blueprint node.

## Testing
- On the Director, untick **Play Tutorial** to start straight at the replant step with the weapon already counted as crafted.
- From the editor console during Play, run:
  `py w=unreal.EditorLevelLibrary.get_game_world(); d=unreal.GameplayStatics.get_all_actors_of_class(w, unreal.MutateoLoopDirector)[0]; d.debug_skip_to_stage(unreal.MutateoLoopStage.WAIT_FOR_CROPS, True)`
- `Saved/mutateo_test.py` teleports you next to an actor and logs the loop state to `Saved/mutateo_test_log.txt`.

## Known gaps / next ideas
- `BP_Plantplot` doesn't stop you planting twice in the same plot, and doesn't set its `Spawnedplant` variable.
- Mutants hit the player using standard Apply Damage. Add **Event AnyDamage** to your character for health and death.
- The scrap pickup by the farm is half-buried in grass, so it's hard to aim at.
- `BP_Mutant` uses the plain Manny mesh. Give it a mutant look, plus attack and death animations through its events (On Attack, On Damaged, On Death).
