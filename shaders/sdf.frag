#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D sdfTexture;
uniform vec3 textColor;
uniform float smoothing = 0.5;

void main() {
    float distance = texture(sdfTexture, TexCoord).r;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    
    FragColor = vec4(texture(sdfTexture, TexCoord).rgb, alpha);
    if (alpha < 0.1) {
        discard;
    }   
}
