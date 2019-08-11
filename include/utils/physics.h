#pragma once

#include <bullet/btBulletDynamicsCommon.h>

enum shapes{BOX,SPHERE};

class Physics
{
public:
    btDiscreteDynamicsWorld* dynamicsWorld;
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btSequentialImpulseConstraintSolver* solver;
    
    Physics()
    {
        this->collisionConfiguration = new btDefaultCollisionConfiguration();
        this->dispatcher = new btCollisionDispatcher(this->collisionConfiguration);
        this->overlappingPairCache = new btDbvtBroadphase();
        this->solver = new btSequentialImpulseConstraintSolver();
        
        this->dynamicsWorld = new btDiscreteDynamicsWorld(this->dispatcher, this->overlappingPairCache, this->solver, this->collisionConfiguration);
        this->dynamicsWorld->setGravity(btVector3(0.0f, -9.82f, 0.0f));
    }
    
    btRigidBody* createRigidBody(int type, glm::vec3 pos, glm::vec3 size, glm::vec3 rot, float m, float friction, float restitution)
    {
        btCollisionShape* cShape = NULL;
        
        btVector3 position = btVector3(pos.x, pos.y, pos.z);
        btQuaternion rotation;
        rotation.setEuler(rot.x, rot.y, rot.z);
        
        if (type == BOX)
        {
            btVector3 dim = btVector3(size.x, size.y, size.z);
            cShape = new btBoxShape(dim);
        }
        else if (type == SPHERE)
            cShape = new btSphereShape(size.x);
            
        this->collisionShapes.push_back(cShape);
        
        btTransform objTransform;
        objTransform.setIdentity();
        objTransform.setRotation(rotation);
        objTransform.setOrigin(position);
        
        btScalar mass = m; // if we want the object to be static, we need to set its mass = 0
        bool isDynamic = (mass != 0.0f); // if the mass is 0, then the object is static, otherwise it is dynamic
        
        btVector3 localInertia(0.0f, 0.0f, 0.0f);
        if (isDynamic)
            cShape->calculateLocalInertia(mass, localInertia);
            
        btDefaultMotionState* motionState = new btDefaultMotionState(objTransform);
        
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, cShape, localInertia);
        rbInfo.m_friction = friction;
        rbInfo.m_restitution = restitution;
        
        if (type == SPHERE)
        {
            rbInfo.m_angularDamping = 0.3f;
            rbInfo.m_rollingFriction = 0.3f;
        }
        
        btRigidBody* body = new btRigidBody(rbInfo);
        this->dynamicsWorld->addRigidBody(body);
        
        return body;
    }
    
    void Clear()
    {
        for (int i = this->dynamicsWorld->getNumCollisionObjects()-1; i>=0; i--)
        {
            btCollisionObject* obj = this->dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            this->dynamicsWorld->removeCollisionObject(obj);
            delete obj;
            
            for (int j = 0; j<this->collisionShapes.size(); j++)
            {
                btCollisionShape* shape = this->collisionShapes[j];
                this->collisionShapes[j] = 0;
                delete shape;
            }
            
            delete this->dynamicsWorld;
            delete this->solver;
            delete this->overlappingPairCache;
            delete this->dispatcher;
            delete this->collisionConfiguration;
            this->collisionShapes.clear();
        }
    }
};