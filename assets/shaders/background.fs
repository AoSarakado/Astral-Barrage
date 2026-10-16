#version 330

uniform vec2 resolution;
uniform float time;
uniform vec3 lightPosition;
uniform vec3 sphereCenter;

out vec4 finalColor;

const float FAR_DISTANCE = 40.0;
const float RAY_EPSILON = 0.002;
const float GROUND_Y = -1.0;
const float MAIN_RADIUS = 0.85;
const vec3 SECOND_CENTER = vec3(1.65, -0.38, -1.2);
const float SECOND_RADIUS = 0.62;
const float LIGHT_RADIUS = 0.55;

// Return the nearest sphere hit farther than the ray origin, if one exists.
float intersectSphere(vec3 rayOrigin, vec3 rayDirection, vec3 center, float radius)
{
    vec3 offset = rayOrigin - center;
    float halfB = dot(offset, rayDirection);
    float discriminant = halfB*halfB - (dot(offset, offset) - radius*radius);
    if (discriminant < 0.0) return FAR_DISTANCE;

    float root = sqrt(discriminant);
    float nearHit = -halfB - root;
    if (nearHit > RAY_EPSILON) return nearHit;

    float farHit = -halfB + root;
    return farHit > RAY_EPSILON ? farHit : FAR_DISTANCE;
}

float intersectGround(vec3 rayOrigin, vec3 rayDirection)
{
    if (rayDirection.y >= -0.0001) return FAR_DISTANCE;
    float hit = (GROUND_Y - rayOrigin.y)/rayDirection.y;
    return hit > RAY_EPSILON ? hit : FAR_DISTANCE;
}

// The fraction of a source circle covered by an occluder circle.
float circleCoverage(float separation, float sourceRadius, float occluderRadius)
{
    // A disk viewed exactly edge-on has zero area in this circle approximation.
    if (sourceRadius == 0.0) return separation < occluderRadius ? 1.0 : 0.0;
    if (separation >= sourceRadius + occluderRadius) return 0.0;
    if (separation <= abs(sourceRadius - occluderRadius))
    {
        if (occluderRadius >= sourceRadius) return 1.0;
        return (occluderRadius*occluderRadius)/(sourceRadius*sourceRadius);
    }

    float sourceSquared = sourceRadius*sourceRadius;
    float occluderSquared = occluderRadius*occluderRadius;
    float separationSquared = separation*separation;
    float sourceAngle = acos(clamp((separationSquared + sourceSquared - occluderSquared)
                                  /(2.0*separation*sourceRadius), -1.0, 1.0));
    float occluderAngle = acos(clamp((separationSquared + occluderSquared - sourceSquared)
                                    /(2.0*separation*occluderRadius), -1.0, 1.0));
    float triangleArea = 0.5*sqrt(max(0.0,
        (-separation + sourceRadius + occluderRadius)
        *(separation + sourceRadius - occluderRadius)
        *(separation - sourceRadius + occluderRadius)
        *(separation + sourceRadius + occluderRadius)));
    float overlapArea = sourceSquared*sourceAngle + occluderSquared*occluderAngle - triangleArea;
    return clamp(overlapArea/(3.14159265*sourceSquared), 0.0, 1.0);
}

// Approximate the sphere and source silhouettes as angular circles at the hit.
// This remains finite where a sphere touches the ground.
float sphereCoverage(vec3 origin, vec3 lightDirection, float lightDistance,
                     float sourceRadius, vec3 center, float radius)
{
    vec3 toSphere = center - origin;
    float sphereDistance = length(toSphere);
    if (sphereDistance <= radius) return 1.0;
    float axisDistance = dot(toSphere, lightDirection);
    if (axisDistance <= 0.0 || axisDistance >= lightDistance) return 0.0;

    float sphereAngle = asin(clamp(radius/sphereDistance, 0.0, 1.0));
    float lightAngle = atan(sourceRadius/lightDistance);
    float separationAngle = acos(clamp(dot(toSphere/sphereDistance, lightDirection), -1.0, 1.0));
    return circleCoverage(separationAngle, lightAngle, sphereAngle);
}

float lightVisibility(vec3 point, vec3 normal, int hitObject)
{
    vec3 origin = point + normal*RAY_EPSILON;
    vec3 toLight = lightPosition - origin;
    float lightDistance = length(toLight);
    vec3 lightDirection = toLight/lightDistance;
    // Equal-area circle for the horizontal disk projected toward this surface.
    float sourceRadius = LIGHT_RADIUS*sqrt(abs(lightDirection.y));

    // Each hit sphere is excluded from its own visibility test.
    float mainCoverage = hitObject == 0 ? 0.0 :
        sphereCoverage(origin, lightDirection, lightDistance,
                       sourceRadius, sphereCenter, MAIN_RADIUS);
    float secondCoverage = hitObject == 1 ? 0.0 :
        sphereCoverage(origin, lightDirection, lightDistance,
                       sourceRadius, SECOND_CENTER, SECOND_RADIUS);

    // The two silhouettes may overlap; independent coverage approximates their union.
    return (1.0 - mainCoverage)*(1.0 - secondCoverage);
}

void main()
{
    vec2 screen = (gl_FragCoord.xy - 0.5*resolution)/resolution.y;

    vec3 rayOrigin = vec3(0.0, 1.5, 5.0);
    vec3 target = vec3(0.0, -0.28, 0.0);
    vec3 forward = normalize(target - rayOrigin);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    vec3 rayDirection = normalize(forward*1.75 + right*screen.x + up*screen.y);

    float mainHit = intersectSphere(rayOrigin, rayDirection, sphereCenter, MAIN_RADIUS);
    float secondHit = intersectSphere(rayOrigin, rayDirection, SECOND_CENTER, SECOND_RADIUS);
    float groundHit = intersectGround(rayOrigin, rayDirection);
    float nearest = min(mainHit, min(secondHit, groundHit));

    // The sky only animates in color; geometry and shadows respond to lightPosition.
    float skyPulse = 0.5 + 0.5*sin(time*0.6);
    vec3 skyBottom = vec3(0.16, 0.22, 0.38);
    vec3 skyTop = vec3(0.025, 0.045, 0.12) + 0.012*skyPulse*vec3(0.3, 0.5, 1.0);
    vec3 color = mix(skyBottom, skyTop, clamp(rayDirection.y*1.3 + 0.45, 0.0, 1.0));

    if (nearest < FAR_DISTANCE)
    {
        vec3 point = rayOrigin + nearest*rayDirection;
        vec3 normal;
        vec3 baseColor;
        int hitObject;

        if (mainHit <= secondHit && mainHit <= groundHit)
        {
            normal = normalize(point - sphereCenter);
            baseColor = vec3(0.33, 0.72, 0.98);
            hitObject = 0;
        }
        else if (secondHit <= groundHit)
        {
            normal = normalize(point - SECOND_CENTER);
            baseColor = vec3(1.0, 0.48, 0.34);
            hitObject = 1;
        }
        else
        {
            normal = vec3(0.0, 1.0, 0.0);
            baseColor = vec3(0.56, 0.55, 0.72);
            hitObject = 2;
        }

        vec3 toLight = lightPosition - point;
        vec3 lightDirection = normalize(toLight);
        float diffuse = max(dot(normal, lightDirection), 0.0);
        float visibility = diffuse > 0.0 ? lightVisibility(point, normal, hitObject) : 0.0;

        // Continuous diffuse lighting keeps the material color and shadow
        // transition clear across both spheres and the ground.
        float illumination = 0.28 + 0.72*diffuse*visibility;
        color = baseColor*illumination;

        if (groundHit <= mainHit && groundHit <= secondHit)
        {
            float distanceFade = 1.0 - smoothstep(7.0, 20.0, nearest);
            color = mix(skyBottom, color, distanceFade);
        }
        else
        {
            float rim = 1.0 - max(dot(normal, -rayDirection), 0.0);
            color += baseColor*0.22*smoothstep(0.65, 0.9, rim);
        }
    }

    finalColor = vec4(color, 1.0);
}
