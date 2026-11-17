#ifdef WORKBENCH
//! Dumps the resolved inventory item catalogs of the mods this addon depends on (RHS USMC, NGSW, DarkGru etc.)
//! to $profile:INF_Flatten/mods/ so candidate items can be reviewed. Skips CIV/FIA/USSR/other factions and INF Arsenal itself.
[WorkbenchPluginAttribute(name: "Dump Dependency Item Catalogs", wbModules: {"ResourceManager"}, awesomeFontCode: 0xF0C5)]
class INF_CatalogDumpPlugin : INF_CatalogFlattenPlugin
{
	protected static const string MODS_DIR = "$profile:INF_Flatten/mods/";
	protected static const ref array<string> SKIP = {"INFArsenal", "INFArsernal_Pub", "_Pub", "CIV", "FIA", "USSR", "ION", "MSV", "FFA", "MEI", "MEC", "Zagoria", "ACE", "Vehicles", "Groups", "Characters", "Tripod"};

	protected int m_iDumped;
	protected int m_iSeen;

	override void Run()
	{
		FileIO.MakeDirectory(OUT_DIR);
		FileIO.MakeDirectory(MODS_DIR);
		Workbench.SearchResources(OnFound, {"conf"}, {"InventoryItems"});
		Print("Dump: done, " + m_iDumped + " catalogs of " + m_iSeen + " matches in " + MODS_DIR, LogLevel.NORMAL);
	}

	protected void OnFound(ResourceName resName, string filePath = "")
	{
		string key = resName;
		m_iSeen++;
		if (!key.Contains("EntityCatalog"))
			return;

		foreach (string skip : SKIP)
		{
			if (key.Contains(skip))
			{
				Print("Dump: skipped " + resName + " (" + skip + ")", LogLevel.NORMAL);
				return;
			}
		}

		Resource res = BaseContainerTools.LoadContainer(resName);
		if (!res || !res.IsValid())
			return;

		string name = FilePath.StripPath(resName);
		string guid = resName.Substring(1, 16);
		string text;
		WriteContainer(res.GetResource().ToBaseContainer(), text, 0, true);
		WriteText(MODS_DIR + guid + "__" + name, text);
		Print("Dump: wrote " + guid + "__" + name + " from " + filePath, LogLevel.NORMAL);
		m_iDumped++;
	}
}
#endif
