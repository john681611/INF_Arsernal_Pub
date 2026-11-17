#ifdef WORKBENCH
//! Writes the fully resolved US item catalog (including everything inherited from parent addons)
//! as standalone .conf files into $profile:INF_Flatten/
[WorkbenchPluginAttribute(name: "Flatten US Item Catalog", wbModules: {"ResourceManager"}, awesomeFontCode: 0xF0C5)]
class INF_CatalogFlattenPlugin : WorkbenchPlugin
{
	protected static const string FACTION_CONF = "{5EB46557DF2AA24F}Configs/Factions/US.conf";
	protected static const string OUT_DIR = "$profile:INF_Flatten/";
	protected static const string PUB_PATH = "Configs/EntityCatalog/Pub/";

	protected int m_iIdCounter;

	override void Run()
	{
		Resource res = BaseContainerTools.LoadContainer(FACTION_CONF);
		if (!res || !res.IsValid())
		{
			Print("Flatten: cannot load " + FACTION_CONF, LogLevel.ERROR);
			return;
		}

		BaseContainer faction = res.GetResource().ToBaseContainer();
		BaseContainerList catalogs = faction.GetObjectArray("m_aEntityCatalogs");
		FileIO.MakeDirectory(OUT_DIR);

		for (int c = 0; c < catalogs.Count(); c++)
		{
			BaseContainerList lists = catalogs.Get(c).GetObjectArray("m_aMultiLists");
			if (!lists)
				continue;

			string topLevel = "SCR_EntityCatalogMultiList {\n m_aMultiLists {\n";
			for (int i = 0; i < lists.Count(); i++)
			{
				BaseContainer entry = lists.Get(i);
				string name = GetFileName(entry, i);
				string text;
				WriteContainer(entry, text, 0, true);
				WriteText(OUT_DIR + name, text);
				topLevel += string.Format("  SCR_EntityCatalogMultiListEntry \"{%1}\" : \"%2%3\" {\n  }\n", NextId(), PUB_PATH, name);
			}
			topLevel += " }\n}\n";
			WriteText(OUT_DIR + "InventoryItems_EntityCatalog_Pub.conf", topLevel);
		}
		Print("Flatten: done, output in " + OUT_DIR, LogLevel.NORMAL);
	}

	protected string GetFileName(BaseContainer entry, int index)
	{
		BaseContainer ancestor = entry.GetAncestor();
		if (ancestor)
		{
			string path = ancestor.GetResourceName();
			path.Replace("\\", "/");
			int slash = path.LastIndexOf("/");
			if (slash >= 0)
				path = path.Substring(slash + 1, path.Length() - slash - 1);
			if (path != string.Empty)
				return path;
		}
		return "Catalog_" + index + ".conf";
	}

	protected void WriteText(string path, string text)
	{
		FileHandle f = FileIO.OpenFile(path, FileMode.WRITE);
		if (!f)
		{
			Print("Flatten: cannot write " + path, LogLevel.ERROR);
			return;
		}
		f.Write(text);
		f.Close();
	}

	//! Known enum/flag variables are written by name; everything else stays numeric
	protected string EnumText(string var, int value)
	{
		switch (var)
		{
			case "m_eItemType": return FlagsToString(SCR_EArsenalItemType, value);
			case "m_eItemMode": return FlagsToString(SCR_EArsenalItemMode, value);
			case "m_eRequiredRank": return typename.EnumToString(SCR_ECharacterRank, value);
		}
		return value.ToString();
	}

	protected string FlagsToString(typename e, int value)
	{
		string single = typename.EnumToString(e, value);
		if (single != string.Empty)
			return single;

		string result;
		for (int bit = 0; bit < 31; bit++)
		{
			int flag = 1 << bit;
			if (!(value & flag))
				continue;
			if (result != string.Empty)
				result += "|";
			result += typename.EnumToString(e, flag);
		}
		if (result == string.Empty)
			return value.ToString();
		return result;
	}

	protected string NextId()
	{
		m_iIdCounter++;
		return string.Format("7A1F%1", (0x1000000000 + m_iIdCounter).ToString());
	}

	protected string Indent(int depth)
	{
		string s;
		for (int i = 0; i < depth; i++)
			s += " ";
		return s;
	}

	//! Writes a container with every variable that is set (own or inherited) so the output has no parent
	protected void WriteContainer(BaseContainer cont, out string text, int depth, bool isRoot)
	{
		string pad = Indent(depth);
		if (isRoot)
			text += cont.GetClassName() + " {\n";
		else
			text += string.Format("%1\"{%2}\" {\n", cont.GetClassName() + " ", NextId());

		for (int v = 0; v < cont.GetNumVars(); v++)
		{
			string var = cont.GetVarName(v);
			if (cont.IsVariableSet(var))
				WriteVar(cont, var, cont.GetDataVarType(v), text, depth + 1);
		}
		text += pad + "}\n";
	}

	protected void WriteVar(BaseContainer cont, string var, DataVarType type, out string text, int depth)
	{
		string pad = Indent(depth);
		switch (type)
		{
			case DataVarType.BOOLEAN:
			{
				bool b;
				cont.Get(var, b);
				text += string.Format("%1%2 %3\n", pad, var, b.ToString(true));
				break;
			}
			case DataVarType.INTEGER:
			{
				int i;
				cont.Get(var, i);
				text += string.Format("%1%2 %3\n", pad, var, EnumText(var, i));
				break;
			}
			case DataVarType.VECTOR3:
			{
				vector v;
				cont.Get(var, v);
				text += string.Format("%1%2 %3 %4 %5\n", pad, var, v[0], v[1], v[2]);
				break;
			}
			case DataVarType.SCALAR:
			{
				float f;
				cont.Get(var, f);
				text += string.Format("%1%2 %3\n", pad, var, f);
				break;
			}
			case DataVarType.STRING:
			case DataVarType.RESOURCE_NAME:
			{
				string s;
				cont.Get(var, s);
				text += string.Format("%1%2 \"%3\"\n", pad, var, s);
				break;
			}
			case DataVarType.OBJECT:
			{
				BaseContainer obj = cont.GetObject(var);
				if (obj)
				{
					text += pad + var + " ";
					WriteContainer(obj, text, depth, false);
				}
				break;
			}
			case DataVarType.OBJECT_ARRAY:
			{
				BaseContainerList list = cont.GetObjectArray(var);
				if (!list)
					break;
				text += string.Format("%1%2 {\n", pad, var);
				for (int i = 0; i < list.Count(); i++)
				{
					text += Indent(depth + 1);
					WriteContainer(list.Get(i), text, depth + 1, false);
				}
				text += pad + "}\n";
				break;
			}
			default:
			{
				Print("Flatten: skipped " + var + " (type " + typename.EnumToString(DataVarType, type) + ")", LogLevel.WARNING);
				break;
			}
		}
	}
}
#endif
