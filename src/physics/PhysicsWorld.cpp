#include "PhysicsWorld.h"

#include <algorithm>
#include <cmath>

namespace core::physics
{
	// -----------------------------------------------------------------------
	// Статические коллайдеры
	// -----------------------------------------------------------------------

	void PhysicsWorld::addStaticCollider(const AABB& box)
	{
		m_staticColliders.push_back(box);
	}

	void PhysicsWorld::removeStaticCollider(const AABB& box)
	{
		// Сравниваем по координатам с небольшим эпсилоном
		constexpr float eps = 1e-4f;
		auto it = std::remove_if(m_staticColliders.begin(), m_staticColliders.end(),
			[&](const AABB& b)
			{
				return glm::all(glm::lessThan(glm::abs(b.min - box.min), glm::vec3(eps))) &&
				       glm::all(glm::lessThan(glm::abs(b.max - box.max), glm::vec3(eps)));
			});
		m_staticColliders.erase(it, m_staticColliders.end());
	}

	// -----------------------------------------------------------------------
	// Коллизионные запросы
	// -----------------------------------------------------------------------

	bool PhysicsWorld::overlapsAny(const AABB& box) const noexcept
	{
		for (const auto& collider : m_staticColliders)
		{
			if (box.overlaps(collider)) return true;
		}
		return false;
	}

	// -----------------------------------------------------------------------
	// Sweep-движение (ось за осью)
	//
	// Порядок X → Z → Y важен:
	//   Y последним, чтобы onGround определялся точно после горизонтального
	//   скольжения вдоль стен.
	// -----------------------------------------------------------------------

	glm::vec3 PhysicsWorld::sweepMove(const AABB& box,
	                                   const glm::vec3& delta,
	                                   bool& onGround) const noexcept
	{
		onGround = false;
		glm::vec3 resolved{ 0.0f };

		// --- Ось X ---
		resolved.x = resolveAxis(box, delta.x, 0);

		// Сдвигаем AABB по X для следующей проверки
		AABB boxAfterX = box.translated({ resolved.x, 0.0f, 0.0f });

		// --- Ось Z ---
		resolved.z = resolveAxis(boxAfterX, delta.z, 2);

		AABB boxAfterXZ = boxAfterX.translated({ 0.0f, 0.0f, resolved.z });

		// --- Ось Y ---
		float resolvedY = resolveAxis(boxAfterXZ, delta.y, 1);

		// Если хотели двигаться вниз, но Y был скорректирован → стоим на земле
		if (delta.y < 0.0f && std::abs(resolvedY) < std::abs(delta.y))
		{
			onGround = true;
		}

		resolved.y = resolvedY;

		return resolved;
	}

	// -----------------------------------------------------------------------
	// Разрешение по одной оси
	// -----------------------------------------------------------------------

	float PhysicsWorld::resolveAxis(const AABB& box, float delta, int axis) const noexcept
	{
		if (std::abs(delta) < 1e-6f) return 0.0f;

		// Строим swept-AABB: объединение текущего и смещённого положения
		glm::vec3 movement{ 0.0f };
		movement[axis] = delta;

		AABB swept;
		swept.min = glm::min(box.min, box.min + movement);
		swept.max = glm::max(box.max, box.max + movement);

		// Проверяем только те коллайдеры, что попадают в swept-регион
		for (const auto& collider : m_staticColliders)
		{
			if (!swept.overlaps(collider)) continue;

			// Глубина проникновения по данной оси
			if (delta > 0.0f)
			{
				// Движение в плюс: упираемся в min коллайдера
				float overlap = collider.min[axis] - box.max[axis];
				if (overlap < delta)
				{
					delta = std::max(0.0f, overlap);
				}
			}
			else
			{
				// Движение в минус: упираемся в max коллайдера
				float overlap = collider.max[axis] - box.min[axis];
				if (overlap > delta)
				{
					delta = std::min(0.0f, overlap);
				}
			}
		}

		return delta;
	}

} // namespace core::physics
