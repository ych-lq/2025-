 import cv2
import numpy as np
from scipy.fftpack import dct, idct


def generate_gradient_image(size=(256, 256), output_path="host_image.jpg"):
    width, height = size
    gradient = np.tile(np.arange(height, dtype=np.uint8), (width, 1))
    cv2.imwrite(output_path, gradient)
    return output_path


def generate_watermark_image(
    text: str = 'W',
    size: tuple = (64, 64),
    output_path: str = "watermark.png",
    font_scale: float = 2.0,
    thickness: int = 3
) -> str:
   
    watermark = np.zeros(size, dtype=np.uint8)
    (text_size, baseline) = cv2.getTextSize(
        text, cv2.FONT_HERSHEY_SIMPLEX, font_scale, thickness
    )
    text_width, text_height = text_size
    x = max((size[0] - text_width) // 2, 0)
    y = max((size[1] + text_height) // 2, 0)
    cv2.putText(
        watermark,
        text,
        (x, y),
        cv2.FONT_HERSHEY_SIMPLEX,
        font_scale,
        255,
        thickness,
    )
    cv2.imwrite(output_path, watermark)
    return output_path


def embed_watermark(
    host_path: str,
    watermark_path: str,
    alpha: float = 10.0,
) -> np.ndarray:

    host = cv2.imread(host_path, cv2.IMREAD_GRAYSCALE)
    watermark = cv2.imread(watermark_path, cv2.IMREAD_GRAYSCALE)
    watermark = cv2.resize(watermark, (64, 64))
    wm_bin = (watermark > 128).astype(np.float32)

    host_dct = dct(dct(host.astype(np.float32), axis=0), axis=1)
    host_dct[1 : 65, 1 : 65] += alpha * wm_bin
    watermarked = idct(idct(host_dct, axis=1), axis=0)
    watermarked = np.clip(watermarked, 0, 255).astype(np.uint8)
    return watermarked


def save_image(image: np.ndarray, output_path: str) -> None:
    cv2.imwrite(output_path, image)
    print(f"Watermark embedded and saved to {output_path}")


def main() -> None:
    host_path = generate_gradient_image()
    watermark_path = generate_watermark_image()
    output_path = "watermarked_image.jpg"
    watermarked_img = embed_watermark(host_path, watermark_path)
    save_image(watermarked_img, output_path)


if __name__ == "__main__":
    main()
