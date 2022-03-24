#pragma once

namespace client_fw
{
	class AssetManager;
	class Mesh;
	class Material;
	class ExternalTexture;
	class RenderTexture;

	class AssetStore final
	{
	public:
		static SPtr<Mesh> LoadMesh(const std::string& path);

		//if .mtl file (parent releative path + mtl's newmtl name)
		static SPtr<Material> LoadMaterial(const std::string& mtl_path);		

		//if .mtl file (path = releative path, mtl_name = mtl's newmtl name)
		static SPtr<Material> LoadMaterial(const std::string& path, const std::string& mtl_name);

		//if .mtl file (releative path)
		static std::map<std::string, SPtr<Material>> LoadMaterials(const std::string& path);
		static SPtr<ExternalTexture> LoadTexture(const std::string& path);

	private:
		friend AssetManager;
		static AssetManager* s_asset_manager;
	};
}


