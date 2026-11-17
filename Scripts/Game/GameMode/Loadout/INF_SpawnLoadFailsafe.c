// WCS_LoadoutEditor is expected to load a player's saved kit from SCR_PlayerArsenalLoadout.OnLoadoutSpawned. With this
// mod stack that call is not reached, so characters spawned from a WCS slot stay naked. This file notes whether
// WCS's spawn load ran and, if it did not, runs it. When WCS's own path works nothing extra happens.
modded class SCR_ArsenalManagerComponent
{
	protected ref map<int, string> m_mINFSpawnLoaded = new map<int, string>();

	override void WCS_LoadPlayerArsenalLoadoutFromSpawn_S(string slotId, int playerId, GameEntity characterEntity)
	{
		if (characterEntity)
			m_mINFSpawnLoaded.Set(playerId, characterEntity.GetID().ToString());
		super.WCS_LoadPlayerArsenalLoadoutFromSpawn_S(slotId, playerId, characterEntity);
	}

	bool INF_SpawnLoadRan(int playerId, GameEntity characterEntity)
	{
		string id;
		if (!characterEntity || !m_mINFSpawnLoaded.Find(playerId, id))
			return false;
		return id == characterEntity.GetID().ToString();
	}
}

// The class attribute must be repeated, or the class stops loading from the loadout manager prefabs.
[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
modded class SCR_PlayerArsenalLoadout
{
	override void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
		super.OnLoadoutSpawned(pOwner, playerId);

		SCR_ArsenalManagerComponent arsenalManager;
		GameEntity playerEntity = GameEntity.Cast(pOwner);
		if (m_sWCSSlotId.IsEmpty() || !playerEntity || !SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;

		if (arsenalManager.INF_SpawnLoadRan(playerId, playerEntity))
			return;

		PrintFormat("[INF_SpawnLoad] WCS did not load the kit on spawn; loading slot '%1' for player %2.", m_sWCSSlotId, playerId, level: LogLevel.WARNING);
		arsenalManager.WCS_LoadPlayerArsenalLoadoutFromSpawn_S(m_sWCSSlotId, playerId, playerEntity);
	}
}
