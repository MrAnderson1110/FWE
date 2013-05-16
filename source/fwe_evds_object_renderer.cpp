////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///   - Redistributions of source code must retain the above copyright
///     notice, this list of conditions and the following disclaimer.
///   - Redistributions in binary form must reproduce the above copyright
///     notice, this list of conditions and the following disclaimer in the
///     documentation and/or other materials provided with the distribution.
///   - Neither the name of the author nor the names of the contributors may
///     be used to endorse or promote products derived from this software without
///     specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
/// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
/// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
/// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////
#include <evds.h>
#include "fwe_evds.h"
#include "fwe_evds_object.h"
#include "fwe_evds_object_renderer.h"
#include "fwe_evds_glwidget.h"

using namespace EVDS;


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
ObjectRenderer::ObjectRenderer(Object* in_object) {
	object = in_object;

	//Create meshes
	glcMesh = new GLC_Mesh();
	glcInstance = new GLC_3DViewInstance(glcMesh);

	//Create mesh generators
	lodMeshGenerator = new ObjectLODGenerator(object,6);
	connect(lodMeshGenerator, SIGNAL(signalLODsReady()), this, SLOT(lodMeshesGenerated()), Qt::QueuedConnection);
	lodMeshGenerator->start();

	//Create initial data
	meshChanged();
	positionChanged();
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
ObjectRenderer::~ObjectRenderer() {
	delete glcInstance;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
void ObjectRenderer::positionChanged() {
	//Offset GLC instance relative to objects parent
	Object* parent = object->getParent();
	if (parent) {
		if (parent->getRenderer()) {
			glcInstance->setMatrix(parent->getRenderer()->getInstance()->matrix());
		} else {
			glcInstance->resetMatrix();
		}

		EVDS_OBJECT* obj = object->getEVDSObject();
		EVDS_STATE_VECTOR vector;
		EVDS_Object_GetStateVector(obj,&vector);
		glcInstance->translate(vector.position.x,vector.position.y,vector.position.z);

		//Add/replace in GL widget to update position
		GLWidget* glview = object->getEVDSEditor()->getGLWidget();
		if (glview->getCollection()->contains(glcInstance->id())) {
			glview->getCollection()->remove(glcInstance->id());			
		}
		glview->getCollection()->add(*(glcInstance));
	}

	//Update position of all children
	for (int i = 0; i < object->getChildrenCount(); i++) {
		object->getChild(i)->getRenderer()->positionChanged();
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
void ObjectRenderer::meshChanged() {
	EVDS_MESH* mesh;

	//Create temporary object
	EVDS_OBJECT* temp_object;
	EVDS_Object_CopySingle(object->getEVDSObject(),0,&temp_object);
	EVDS_Object_Initialize(temp_object,1);

	//Ask dear generator LOD thing to generate LODs
	lodMeshGenerator->updateMesh();

	//Do the quick hack job anyway
	glcMesh->clear();
	//for (int i = 0; i < 4; i++) {
		//EVDS_Mesh_Generate(temp_object,&mesh,getLODResolution(3-i),EVDS_MESH_USE_DIVISIONS);
		//addLODMesh(mesh,i);
		//EVDS_Mesh_Destroy(mesh);
	//}

	EVDS_Mesh_Generate(temp_object,&mesh,32.0f,EVDS_MESH_USE_DIVISIONS);
	addLODMesh(mesh,0);
	EVDS_Mesh_Destroy(mesh);
	//EVDS_Mesh_Generate(temp_object,&mesh,24.0f,EVDS_MESH_USE_DIVISIONS);
	//addLODMesh(mesh,1);
	//EVDS_Mesh_Destroy(mesh);

	glcMesh->finish();
	EVDS_Object_Destroy(temp_object);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
void ObjectRenderer::addLODMesh(EVDS_MESH* mesh, int lod) {
	if (!mesh) return;

	//Add empty mesh?
	if (mesh->num_triangles == 0) {
		GLfloatVector verticesVector;
		GLfloatVector normalsVector;
		IndexList indicesList;

		verticesVector << 0 << 0 << 0;
		normalsVector << 0 << 0 << 0;
		indicesList << 0 << 0 << 0;

		glcMesh->addVertice(verticesVector);
		glcMesh->addNormals(normalsVector);
		glcMesh->addTriangles(new GLC_Material(), indicesList, lod);
	} else {
		GLfloatVector verticesVector;
		GLfloatVector normalsVector;
		IndexList indicesList;
		GLC_Material* glcMaterial = new GLC_Material();

		if (object->getType() == "fuel_tank") {
			if (object->isOxidizerTank()) {
				glcMaterial->setDiffuseColor(QColor(0,0,255));
			} else {
				glcMaterial->setDiffuseColor(QColor(255,255,0));
			}
		}

		//Add all data
		int firstVertexIndex = glcMesh->VertexCount();	
		for (int i = 0; i < mesh->num_vertices; i++) {
			verticesVector << mesh->vertices[i].x;
			verticesVector << mesh->vertices[i].y;
			verticesVector << mesh->vertices[i].z;
			normalsVector << mesh->normals[i].x;
			normalsVector << mesh->normals[i].y;
			normalsVector << mesh->normals[i].z;
		}
		for (int i = 0; i < mesh->num_triangles; i++) {
			if ((mesh->triangles[i].vertex[0].y > 0.0) &&
				(mesh->triangles[i].vertex[1].y > 0.0) &&
				(mesh->triangles[i].vertex[2].y > 0.0)) {
				indicesList << mesh->triangles[i].indices[0] + firstVertexIndex;
				indicesList << mesh->triangles[i].indices[1] + firstVertexIndex;
				indicesList << mesh->triangles[i].indices[2] + firstVertexIndex;
			}
		}
		indicesList << 0 << 0 << 0;

		glcMesh->addVertice(verticesVector);
		glcMesh->addNormals(normalsVector);
		glcMesh->addTriangles(glcMaterial, indicesList, lod); //coarse_mesh->resolution);
		//glcMesh->reverseNormals();
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
void ObjectRenderer::lodMeshesGenerated() {
	printf("LODs ready %p\n",this);
	
	glcMesh->clear();
	for (int i = 0; i < lodMeshGenerator->getNumLODs(); i++) {
		addLODMesh(lodMeshGenerator->getMesh(i),i);
	}
	glcMesh->finish();
	object->getEVDSEditor()->updateObject(NULL); //Force into repaint
}




////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
float ObjectLODGenerator::getLODResolution(int lod) {
	return 32.0f + 32.0f * lod;
	//return 1.0f / (1.0f + 2*lod);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
ObjectLODGenerator::ObjectLODGenerator(Object* in_object, int in_lods) {
	object = in_object;
	numLods = in_lods;

	//Initialize temporary object
	object_copy = 0;
	mesh = (EVDS_MESH**)malloc(numLods*sizeof(EVDS_MESH*));
	for (int i = 0; i < numLods; i++) mesh[i] = 0;

	//Delete thread when work is finished
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));	
	doStopWork = false;
	needMesh = false; 
	meshCompleted = true;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
EVDS_MESH* ObjectLODGenerator::getMesh(int lod) { 
	if ((!needMesh) && mesh && meshCompleted && this->isRunning()) { 
		return mesh[lod];
	} else {
		return 0;
	} 
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get a temporary copy of the rendered object
////////////////////////////////////////////////////////////////////////////////
void ObjectLODGenerator::updateMesh() {
	needMesh = true;
	if (this->isRunning()) {
		readingLock.lock();
		EVDS_Object_CopySingle(object->getEVDSObject(),0,&object_copy);
		readingLock.unlock();
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief
////////////////////////////////////////////////////////////////////////////////
void ObjectLODGenerator::run() {
	while (!doStopWork) {
		if (needMesh) {
			readingLock.lock();

			//Start making the mesh
			needMesh = false;
			meshCompleted = false;

			//Transfer and initialize work object
			EVDS_OBJECT* work_object = object_copy; //Fetch the pointer
			EVDS_Object_TransferInitialization(work_object); //Get rights to work with variables
			EVDS_Object_Initialize(work_object,1);
			object_copy = 0;

			for (int lod = 0; lod < numLods; lod++) {
				printf("Generating mesh %p for level %d\n",object,lod);
			
				//Remove old mesh
				if (mesh[lod]) {
					EVDS_Mesh_Destroy(mesh[lod]);
					mesh[lod] = 0;
				}

				//Check if job must be aborted
				if (needMesh) {
					printf("Aborted job early\n");
					break;
				}

				//Create new one
				EVDS_Mesh_Generate(work_object,&mesh[lod],getLODResolution(numLods-lod-1),EVDS_MESH_USE_DIVISIONS);
				printf("Done mesh %p %p for level %d\n",object,mesh,lod);
			}

			//Release the object that was worked on
			if (work_object) {
				EVDS_Object_Destroy(work_object);
			}
			readingLock.unlock();

			//Finish working
			meshCompleted = true;

			//If new mesh is needed, do not return generated one - return actually needed one instead
			if (!needMesh) {
				emit signalLODsReady();
			}
		}
		msleep(100);
	}

	//Finish thread work and destroy HQ mesh
	//if (mesh) EVDS_Mesh_Destroy(mesh);
}