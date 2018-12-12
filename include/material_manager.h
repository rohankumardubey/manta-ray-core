#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

#include <vector>

namespace manta {

	class Material;

	class MaterialManager {
	public:
		MaterialManager();
		~MaterialManager();

		template <typename T>
		T *newMaterial() {
			T *newMaterial = new T;
			newMaterial->setIndex(m_currentIndex);
			m_materials.push_back(newMaterial);
			m_currentIndex++;

			return newMaterial;
		}

		Material *getMaterial(int index) const { return m_materials[index]; }

	protected:
		std::vector<Material *> m_materials;
		int m_currentIndex;
	};

}

#endif /* MATERIAL_H */