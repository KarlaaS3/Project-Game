#include "EntityManager.h"
#include "Entity.h"

#include <algorithm>
#include <ranges>

EntityManager::EntityManager()
	: m_totalEntities(0)
{}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	// create a new Entity object
	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

	// store it in entities vector
	m_EntitiesToAdd.push_back(entity);

	// return shared pointer to it 
	return entity;
}


EntityVec& EntityManager::getEntities()
{
	return m_entities;
}


EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}


void EntityManager::removeDeadEntities(EntityVec& v)
{
	v.erase(std::remove_if(v.begin(), v.end(), [](auto e) {return!(e->isActive()); }), v.end());
}


void EntityManager::update()
{
	// Remove dead entities 
	removeDeadEntities(m_entities);
	for (auto& [_, entityVec] : m_entityMap)
		removeDeadEntities(entityVec);


	// add new entities
	for (auto e : m_EntitiesToAdd)
	{
		m_entities.push_back(e);
		m_entityMap[e->getTag()].push_back(e);
	}
	m_EntitiesToAdd.clear();
}
