#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

// Global değişkenler
const int kenarSayisi = 6;
const int araNoktaSayisi = 4; // Her kenar için ara nokta sayısı

// Pencere boyutları
const unsigned int GENISLIK = 800;
const unsigned int YUKSEKLIK = 600;

// Fare kontrolü için değişkenler
float donusX = 0.0f;
float donusY = 0.0f;
bool fareBasili = false;
double sonFareX, sonFareY;
float cubukDegeri = 0.0f; // 0.0 (altıgen) - 1.0 (silindir)
bool cubukSecili = false;

// Vertex shader
const char* vertexShaderKodu = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Fragment shader
const char* fragmentShaderKodu = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.5, 0.5, 1.0, 1.0);
    }
)";

// Çubuk için vertex ve fragment shader ekleyelim
const char* cubukVertexShaderKodu = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

const char* cubukFragmentShaderKodu = R"(
    #version 330 core
    uniform vec3 renk;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(renk, 1.0);
    }
)";

// Altıgen prizma vertex'lerini oluştur
std::vector<float> altigenPrizmaOlustur(float yaricap, float yukseklik, float yumusatmaFaktoru) {
    std::vector<float> vertexler;
    
    // Yan yüzeyler için vertexler
    for(int i = 0; i <= kenarSayisi; i++) {
        float aci = (float)i * 2.0f * M_PI / kenarSayisi;
        float sonrakiAci = (float)((i + 1) % kenarSayisi) * 2.0f * M_PI / kenarSayisi;
        
        for(int j = 0; j <= araNoktaSayisi; j++) {
            float t = (float)j / araNoktaSayisi;
            float araAci = glm::mix(aci, sonrakiAci, t);
            float r = yaricap;
            
            // Yumuşatma faktörüne göre yarıçapı ayarla - DEĞİŞTİRİLDİ
            if(yumusatmaFaktoru > 0.0f) {
                // Altıgenden silindire doğru yumuşatma
                float koseliFaktor = yaricap * sin(araAci * kenarSayisi) / kenarSayisi;
                r = yaricap - yumusatmaFaktoru * koseliFaktor; // Değiştirilen kısım
            }
            
            float x = r * cos(araAci);
            float z = r * sin(araAci);
            
            // Üst vertex
            vertexler.push_back(x);
            vertexler.push_back(yukseklik/2);
            vertexler.push_back(z);
            
            // Alt vertex
            vertexler.push_back(x);
            vertexler.push_back(-yukseklik/2);
            vertexler.push_back(z);
        }
    }
    
    // Üst yüzey için vertexler
    for(int i = 0; i < kenarSayisi; i++) {
        // Merkez nokta
        vertexler.push_back(0.0f);
        vertexler.push_back(yukseklik/2);
        vertexler.push_back(0.0f);
        
        float aci = (float)i * 2.0f * M_PI / kenarSayisi;
        float sonrakiAci = (float)((i + 1) % kenarSayisi) * 2.0f * M_PI / kenarSayisi;
        
        float r = yaricap;
        // Yumuşatma faktörüne göre yarıçapı ayarla - DEĞİŞTİRİLDİ
        if(yumusatmaFaktoru > 0.0f) {
            float koseliFaktor = yaricap * sin(aci * kenarSayisi) / kenarSayisi;
            r = yaricap - yumusatmaFaktoru * koseliFaktor; // Değiştirilen kısım
        }
        
        // İlk kenar noktası
        float x1 = r * cos(aci);
        float z1 = r * sin(aci);
        vertexler.push_back(x1);
        vertexler.push_back(yukseklik/2);
        vertexler.push_back(z1);
        
        // İkinci kenar noktası
        r = yaricap;
        // Yumuşatma faktörüne göre yarıçapı ayarla - DEĞİŞTİRİLDİ
        if(yumusatmaFaktoru > 0.0f) {
            float koseliFaktor = yaricap * sin(sonrakiAci * kenarSayisi) / kenarSayisi;
            r = yaricap - yumusatmaFaktoru * koseliFaktor; // Değiştirilen kısım
        }
        float x2 = r * cos(sonrakiAci);
        float z2 = r * sin(sonrakiAci);
        vertexler.push_back(x2);
        vertexler.push_back(yukseklik/2);
        vertexler.push_back(z2);
    }
    
    // Alt yüzey için vertexler
    for(int i = 0; i < kenarSayisi; i++) {
        // Merkez nokta
        vertexler.push_back(0.0f);
        vertexler.push_back(-yukseklik/2);
        vertexler.push_back(0.0f);
        
        float aci = (float)i * 2.0f * M_PI / kenarSayisi;
        float sonrakiAci = (float)((i + 1) % kenarSayisi) * 2.0f * M_PI / kenarSayisi;
        
        float r = yaricap;
        if(yumusatmaFaktoru > 0.0f) {
            float koseliFaktor = yaricap * sin(sonrakiAci * kenarSayisi) / kenarSayisi;
            r = yaricap - yumusatmaFaktoru * koseliFaktor; // Değiştirilen kısım
        }
        
        // İlk kenar noktası (alt yüzey için saat yönünün tersine)
        float x1 = r * cos(sonrakiAci);
        float z1 = r * sin(sonrakiAci);
        vertexler.push_back(x1);
        vertexler.push_back(-yukseklik/2);
        vertexler.push_back(z1);
        
        // İkinci kenar noktası
        r = yaricap;
        if(yumusatmaFaktoru > 0.0f) {
            float koseliFaktor = yaricap * sin(aci * kenarSayisi) / kenarSayisi;
            r = yaricap - yumusatmaFaktoru * koseliFaktor; // Değiştirilen kısım
        }
        float x2 = r * cos(aci);
        float z2 = r * sin(aci);
        vertexler.push_back(x2);
        vertexler.push_back(-yukseklik/2);
        vertexler.push_back(z2);
    }
    
    return vertexler;
}

// Fare hareket callback fonksiyonu
void fareHareketCallback(GLFWwindow* pencere, double xpos, double ypos) {
    if(fareBasili) {
        float xfark = float(xpos - sonFareX);
        float yfark = float(ypos - sonFareY);
        
        donusX += yfark * 0.5f;
        donusY += xfark * 0.5f;
    }
    
    // Çubuk kontrolü
    if(cubukSecili) {
        cubukDegeri = glm::clamp((float)(xpos - 200) / 400.0f, 0.0f, 1.0f);
    }
    
    sonFareX = xpos;
    sonFareY = ypos;
}

// Fare tuş callback fonksiyonu
void fareTusCallback(GLFWwindow* pencere, int tus, int aksiyon, int modlar) {
    if(tus == GLFW_MOUSE_BUTTON_LEFT) {
        if(aksiyon == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(pencere, &xpos, &ypos);
            
            // Çubuk alanı kontrolü
            if(ypos > YUKSEKLIK - 50 && ypos < YUKSEKLIK - 30 &&
               xpos > 200 && xpos < 600) {
                cubukSecili = true;
            } else {
                fareBasili = true;
            }
        } else if(aksiyon == GLFW_RELEASE) {
            fareBasili = false;
            cubukSecili = false;
        }
    }
}

int main() {
    // GLFW başlatma
    if(!glfwInit()) {
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* pencere = glfwCreateWindow(GENISLIK, YUKSEKLIK, "Altıgen Prizma", NULL, NULL);
    if(!pencere) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(pencere);
    glfwSetCursorPosCallback(pencere, fareHareketCallback);
    glfwSetMouseButtonCallback(pencere, fareTusCallback);
    
    // GLEW başlatma
    if(glewInit() != GLEW_OK) {
        return -1;
    }
    
    // Shader programı oluştur
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderKodu, NULL);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderKodu, NULL);
    glCompileShader(fragmentShader);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Çubuk için shader programı oluştur
    unsigned int cubukVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cubukVertexShader, 1, &cubukVertexShaderKodu, NULL);
    glCompileShader(cubukVertexShader);
    
    unsigned int cubukFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cubukFragmentShader, 1, &cubukFragmentShaderKodu, NULL);
    glCompileShader(cubukFragmentShader);
    
    unsigned int cubukShaderProgram = glCreateProgram();
    glAttachShader(cubukShaderProgram, cubukVertexShader);
    glAttachShader(cubukShaderProgram, cubukFragmentShader);
    glLinkProgram(cubukShaderProgram);
    
    glDeleteShader(cubukVertexShader);
    glDeleteShader(cubukFragmentShader);
    
    // VAO ve VBO oluştur
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    // Çubuk için vertex verileri
    float cubukVertexler[] = {
        // Ana çubuk
        -0.75f, -0.9f,
         0.75f, -0.9f,
         0.75f, -0.85f,
        -0.75f, -0.85f,
        // Hareketli kısım
        -0.05f, -0.9f,
         0.05f, -0.9f,
         0.05f, -0.85f,
        -0.05f, -0.85f
    };
    
    unsigned int cubukVAO, cubukVBO;
    glGenVertexArrays(1, &cubukVAO);
    glGenBuffers(1, &cubukVBO);
    
    glBindVertexArray(cubukVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubukVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubukVertexler), cubukVertexler, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Ana döngü
    while(!glfwWindowShouldClose(pencere)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        // Vertex verilerini güncelle
        std::vector<float> vertexler = altigenPrizmaOlustur(0.5f, 1.0f, cubukDegeri);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexler.size() * sizeof(float), vertexler.data(), GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Shader programını kullan
        glUseProgram(shaderProgram);
        
        // Matris dönüşümlerini ayarla
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(donusX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(donusY), glm::vec3(0.0f, 1.0f, 0.0f));
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)GENISLIK / (float)YUKSEKLIK, 0.1f, 100.0f);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Çiz
        glDrawArrays(GL_TRIANGLE_STRIP, 0, (kenarSayisi + 1) * (araNoktaSayisi + 1) * 2);
        
        // Üst yüzey için
        int yanYuzeyVertexSayisi = (kenarSayisi + 1) * (araNoktaSayisi + 1) * 2;
        glDrawArrays(GL_TRIANGLES, yanYuzeyVertexSayisi, kenarSayisi * 3);
        
        // Alt yüzey için
        glDrawArrays(GL_TRIANGLES, yanYuzeyVertexSayisi + kenarSayisi * 3, kenarSayisi * 3);
        
        // Çubuğu çiz
        glDisable(GL_DEPTH_TEST);
        glUseProgram(cubukShaderProgram);
        glBindVertexArray(cubukVAO);
        
        // Ana çubuğu çiz
        glUniform3f(glGetUniformLocation(cubukShaderProgram, "renk"), 0.7f, 0.7f, 0.7f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        // Hareketli kısmı çiz
        glm::mat4 cubukModel = glm::mat4(1.0f);
        cubukModel = glm::translate(cubukModel, glm::vec3(cubukDegeri * 1.5f, 0.0f, 0.0f));
        glUniform3f(glGetUniformLocation(cubukShaderProgram, "renk"), 0.3f, 0.3f, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        
        glfwSwapBuffers(pencere);
        glfwPollEvents();
    }
    
    // Temizlik
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &cubukVAO);
    glDeleteBuffers(1, &cubukVBO);
    glDeleteProgram(cubukShaderProgram);
    
    glfwTerminate();
    return 0;
}
