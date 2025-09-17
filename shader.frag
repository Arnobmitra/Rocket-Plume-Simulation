#version 330 core
out vec4 FragColor;
uniform float u_time;

void main() {
    float y = gl_FragCoord.y / 600.0;
    float intensity = exp(-y * 5.0) + 0.3 * sin(u_time * 5.0 + y * 20.0);
    vec3 color = vec3(1.0, 0.5, 0.0) * intensity;
    FragColor = vec4(color, 1.0);
}