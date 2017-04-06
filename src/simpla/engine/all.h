//
// Created by salmon on 17-3-19.
//

#ifndef SIMPLA_ENGINE_ALL_H
#define SIMPLA_ENGINE_ALL_H

#include "Atlas.h"
#include "Attribute.h"
#include "Context.h"
#include "Domain.h"

#include "Mesh.h"
#include "Model.h"
#include "Patch.h"
#include "Task.h"
#include "TimeIntegrator.h"
/**
 * @startuml
 * GeoObject "n" --* "1" Material
 * Material "1" *-- "1" Worker
 * Mesh   "m"  o-- "n" Patch
 * Worker   "n" *-- "1" Mesh
 * Worker   "1" *-- "n" Task
 * @enduml
 *
 */
#endif  // SIMPLA_ENGINE_ALL_H
