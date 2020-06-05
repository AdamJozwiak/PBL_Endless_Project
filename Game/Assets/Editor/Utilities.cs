using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

public static class EditorUtils
{
    [MenuItem("Utils/Reserialize all prefabs")]
    private static void onClick_ReserializeAllPrefabs()
    {
        foreach (string _prefabPath in GetAllPrefabs())
        {
            GameObject _prefabAsset = AssetDatabase.LoadAssetAtPath<GameObject>(_prefabPath);
            if (!PrefabUtility.IsPartOfImmutablePrefab(_prefabAsset))
            {
                PrefabUtility.SavePrefabAsset(_prefabAsset);
            }
        }
    }

    public static string[] GetAllPrefabs()
    {
        List<string> _prefabPaths = new List<string>();
        foreach (string _paths in AssetDatabase.GetAllAssetPaths())
        {
            if (_paths.Contains(".prefab"))
            {
                _prefabPaths.Add(_paths);
            }
        }
        return _prefabPaths.ToArray();
    }
}
