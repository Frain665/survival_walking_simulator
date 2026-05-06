#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace core::physics
{
	// -----------------------------------------------------------------------
	// AABB — ось-выровненный параллелепипед (самый быстрый тип коллизии)
	// -----------------------------------------------------------------------
	struct AABB
	{
		glm::vec3 min{ 0.0f };
		glm::vec3 max{ 0.0f };

		[[nodiscard]] bool overlaps(const AABB& other) const noexcept
		{
			return (min.x < other.max.x && max.x > other.min.x) &&
			       (min.y < other.max.y && max.y > other.min.y) &&
			       (min.z < other.max.z && max.z > other.min.z);
		}

		[[nodiscard]] bool contains(const glm::vec3& point) const noexcept
		{
			return point.x >= min.x && point.x <= max.x &&
			       point.y >= min.y && point.y <= max.y &&
			       point.z >= min.z && point.z <= max.z;
		}

		// Смещение AABB в пространстве (для движущегося объекта)
		[[nodiscard]] AABB translated(const glm::vec3& offset) const noexcept
		{
			return { min + offset, max + offset };
		}

		[[nodiscard]] glm::vec3 center() const noexcept { return (min + max) * 0.5f; }
		[[nodiscard]] glm::vec3 size()   const noexcept { return max - min; }
	};

	// -----------------------------------------------------------------------
	// Результат разрешения коллизии
	// -----------------------------------------------------------------------
	struct CollisionResult
	{
		bool      collided   { false };
		glm::vec3 normal     { 0.0f };   // нормаль поверхности (от блока к игроку)
		float     penetration{ 0.0f };   // глубина проникновения
	};

	// -----------------------------------------------------------------------
	// PhysicsWorld
	//
	// Хранит статические коллайдеры мира (блоки чанков) и предоставляет
	// функции для:
	//   - регистрации/удаления блоков
	//   - теста AABB-sweep против мира
	//   - применения гравитации к RigidBody-like структурам
	// -----------------------------------------------------------------------
	class PhysicsWorld
	{
	public:
		// Гравитация (м/с²), можно менять для разных биомов / режимов
		static constexpr float DEFAULT_GRAVITY = -24.0f; // чуть сильнее чем в Minecraft

		explicit PhysicsWorld(float gravity = DEFAULT_GRAVITY) noexcept
			: m_gravity(gravity) {}

		// -------------------------------------------------------
		// Управление статическими коллайдерами
		// -------------------------------------------------------

		/**
		 * @brief Добавить AABB блока/объекта в мир.
		 *        Вызывается когда чанк загружается / блок ставится.
		 */
		void addStaticCollider(const AABB& box);

		/**
		 * @brief Удалить AABB (блок сломан / чанк выгружен).
		 *        O(n) — для игры с небольшим view distance приемлемо.
		 *        В будущем замените на spatial hash / octree.
		 */
		void removeStaticCollider(const AABB& box);

		/** Очистить все коллайдеры (смена уровня, телепорт). */
		void clearColliders() noexcept { m_staticColliders.clear(); }

		// -------------------------------------------------------
		// Коллизионные запросы
		// -------------------------------------------------------

		/**
		 * @brief Проверить, пересекается ли AABB с любым статическим коллайдером.
		 */
		[[nodiscard]] bool overlapsAny(const AABB& box) const noexcept;

		/**
		 * @brief Sweep-тест: сдвинуть AABB на вектор delta и вернуть
		 *        фактически разрешённое смещение с учётом коллизий.
		 *
		 * Алгоритм: сначала по X, потом по Z, потом по Y —
		 * это позволяет скользить вдоль стен и правильно
		 * ставить игрока на землю.
		 *
		 * @param box         AABB объекта в текущей позиции
		 * @param delta       желаемое смещение
		 * @param onGround    [out] true если Y-коллизия снизу (игрок стоит)
		 * @return            фактическое смещение после разрешения
		 */
		[[nodiscard]] glm::vec3 sweepMove(const AABB& box,
		                                   const glm::vec3& delta,
		                                   bool& onGround) const noexcept;

		// -------------------------------------------------------
		// Параметры
		// -------------------------------------------------------
		void  setGravity(float g) noexcept { m_gravity = g; }
		[[nodiscard]] float getGravity() const noexcept { return m_gravity; }

	private:
		// Разрешение по одной оси, возвращает скорректированное смещение по этой оси
		[[nodiscard]] float resolveAxis(const AABB& box,
		                                float        delta,
		                                int          axis) const noexcept;

		std::vector<AABB> m_staticColliders;
		float             m_gravity;
	};

} // namespace core::physics
