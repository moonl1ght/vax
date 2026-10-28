#include "xpbdSolver.h"

using namespace vax::physics;

void XPBDSolver::solve(BodyStorage& bodyStorage, std::vector<ContactConstraint>& contacts, float dt) {
    for (size_t i = 0; i < contacts.size(); ++i) {
        auto& contact = contacts[i];
        auto& firstBody = bodyStorage.dynamicBodies[contact.first];
        const float firstMassInverse = firstBody.rigidBodyComponent.invMass;
        float secondMassInverse = 0.0f;
        glm::vec3 secondPosition, secondPreviousPosition;
        if (contact.secondIsStatic) {
            secondPosition = secondPreviousPosition = bodyStorage.staticBodies[contact.second].position;
        } else {
            auto& secondBody = bodyStorage.dynamicBodies[contact.second];
            secondMassInverse = secondBody.rigidBodyComponent.invMass;
            secondPosition = secondBody.position;
            secondPreviousPosition = secondBody.previousPosition;
        }
        const float inverseMassSum = firstMassInverse + secondMassInverse;
        if (inverseMassSum <= EPSILON) [[unlikely]] {
            continue;
        }

        const glm::vec3& normal = contact.normalFromFirstToSecond;

        const glm::vec3 pointOnFirstBody = firstBody.position + contact.localPointFirst;
        const glm::vec3 pointOnSecondBody = secondPosition + contact.localPointSecond;
        const float d = glm::dot(pointOnFirstBody - pointOnSecondBody, normal);
        if (d <= 0.0f) {
            continue;
        }
        const float alphaTilde = contact.compliance / (dt * dt);
        const float dLambda = (d - alphaTilde * contact.lambdaNormal) / (inverseMassSum + alphaTilde);
        contact.lambdaNormal += dLambda;
        firstBody.position -= normal * (dLambda * firstMassInverse);
        if (!contact.secondIsStatic)
            bodyStorage.dynamicBodies[contact.second].position += normal * (dLambda * secondMassInverse);

        const glm::vec3 dp =
            (firstBody.position - firstBody.previousPosition) - (secondPosition - secondPreviousPosition);
        const glm::vec3 dpT = dp - normal * glm::dot(dp, normal);
        const float dpTLen = glm::length(dpT);
        const float lambdaT = dpTLen / inverseMassSum;
        if (dpTLen > EPSILON && lambdaT < contact.staticFriction * contact.lambdaNormal) {
            firstBody.position -= dpT * (firstMassInverse / inverseMassSum);
            if (!contact.secondIsStatic)
                bodyStorage.dynamicBodies[contact.second].position += dpT * (secondMassInverse / inverseMassSum);
            contact.lambdaTangent += lambdaT;
        }
    }
}

void XPBDSolver::solveVelocities(
    BodyStorage& bodyStorage, const std::vector<ContactConstraint>& contacts, const glm::vec3& gravity, float dt
) {
    const float restitutionThreshold = 2.0f * glm::length(gravity) * dt;

    for (size_t i = 0; i < contacts.size(); ++i) {
        const auto& contact = contacts[i];
        if (contact.lambdaNormal <= 0.0f)
            continue;

        auto& firstBody = bodyStorage.dynamicBodies[contact.first];
        const float firstMassInverse = firstBody.rigidBodyComponent.invMass;
        float secondMassInverse = 0.0f;
        glm::vec3 secondVelocity(0.0f);
        if (!contact.secondIsStatic) {
            const auto& secondBody = bodyStorage.dynamicBodies[contact.second];
            secondMassInverse = secondBody.rigidBodyComponent.invMass;
            secondVelocity = secondBody.velocity;
        }
        const float inverseMassSum = firstMassInverse + secondMassInverse;
        if (inverseMassSum <= EPSILON)
            continue;

        const glm::vec3& normal = contact.normalFromFirstToSecond;
        const glm::vec3 deltaVelocity = firstBody.velocity - secondVelocity;
        const float vn = glm::dot(deltaVelocity, normal);
        const glm::vec3 vt = deltaVelocity - normal * vn;
        const float vtLen = glm::length(vt);

        glm::vec3 dv(0.0f);

        if (vtLen > EPSILON) {
            dv -= vt * (std::min(contact.dynamicFriction * contact.lambdaNormal / dt, vtLen) / vtLen);
        }

        // restitution
        const float e = std::abs(contact.normalVelocityBefore) <= restitutionThreshold ? 0.0f : contact.restitution;
        const float vnTarget = -e * std::max(contact.normalVelocityBefore, 0.0f);
        dv += normal * (vnTarget - vn);

        firstBody.velocity += dv * (firstMassInverse / inverseMassSum);
        if (!contact.secondIsStatic)
            bodyStorage.dynamicBodies[contact.second].velocity -= dv * (secondMassInverse / inverseMassSum);
    }
}