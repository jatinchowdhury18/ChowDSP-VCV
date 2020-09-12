#include <Eigen/Dense>

class GeneralSVF {
public:
    GeneralSVF();

    void reset();
    void calcCoefs(float r, float k, float wc);

    inline float process(float x) {
        Eigen::Matrix<float, 4, 1> v = A_tilde * v_n1 + B_tilde * x;
        float y = (C_tilde * v_n1)(0, 0) + D_tilde(0, 0) * x;
        v_n1 = v;
        return y;
    }

private:
    Eigen::Matrix<float, 4, 4> A;
    Eigen::Matrix<float, 4, 1> B;
    Eigen::Matrix<float, 1, 4> C;

    Eigen::Matrix<float, 4, 4> A_tilde;
    Eigen::Matrix<float, 4, 1> B_tilde;
    Eigen::Matrix<float, 1, 4> C_tilde;
    Eigen::Matrix<float, 1, 1> D_tilde;

    float g;
    Eigen::Matrix<float, 4, 1> v_n1;

    float drive = 1.0f;
    float invDrive = 1.0f;
};
