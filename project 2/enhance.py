from PIL import Image, ImageDraw, ImageFont, ImageEnhance
import os

class WatermarkProcessor:
    def __init__(
        self,
        watermark_text: str,
        position_flag: int,
        opacity: float,
        font_path: str = "cambriab.ttf",
        font_size: int = 35,
    ) -> None:
        self.watermark_text = watermark_text
        self.position_flag = position_flag
        self.opacity = opacity
        self.font = ImageFont.truetype(font_path, size=font_size)

    def _calculate_position(self, img_width: int, img_height: int) -> tuple:
        bbox = self.font.getbbox(self.watermark_text)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        pos_map = {
            1: (0, 0),  # top‑left
            2: (0, img_height - text_height),  # bottom‑left
            3: (img_width - text_width, 0),  # top‑right
            4: (img_width - text_width, img_height - text_height),  # bottom‑right
            5: ((img_width - text_width) // 2, (img_height - text_height) // 2),  # centre
        }
        return pos_map.get(self.position_flag, (0, 0))

    def apply_watermark_to_image(self, image_path: str) -> tuple:

        Returns
        -------
        tuple
            A tuple ``(success: bool, message: str)`` indicating the
            result of the operation.
      
        try:
            with Image.open(image_path).convert('RGBA') as base_image:
                width, height = base_image.size
                # Create an overlay image for the watermark
                overlay = Image.new('RGBA', base_image.size, (255, 255, 255, 0))
                draw = ImageDraw.Draw(overlay)
                # Calculate position and draw text
                pos_x, pos_y = self._calculate_position(width, height)
                draw.text((pos_x, pos_y), self.watermark_text, font=self.font, fill="blue")
                # Adjust opacity via brightness enhancement of the alpha channel
                alpha_channel = overlay.split()[3]
                alpha_channel = ImageEnhance.Brightness(alpha_channel).enhance(self.opacity)
                overlay.putalpha(alpha_channel)
                # Composite overlay onto the base image
                result = Image.alpha_composite(base_image, overlay)
                result.save(image_path)
            return True, f"水印添加成功: {image_path}"
        except Exception as e:
            return False, f"处理失败: {str(e)}"


def process_path(path: str, processor: WatermarkProcessor) -> list:
    results = []
    if os.path.isfile(path) and path.lower().endswith('.png'):
        results.append(processor.apply_watermark_to_image(path))
    elif os.path.isdir(path):
        for filename in os.listdir(path):
            if filename.lower().endswith('.png'):
                file_path = os.path.join(path, filename)
                results.append(processor.apply_watermark_to_image(file_path))
    else:
        results.append((False, '无效路径或非PNG格式'))
    return results


def main() -> None:
    image_path = input('图片路径：').strip()
    watermark_text = input('水印文字：')
    position_flag = int(
        input('水印位置（1：左上角，2：左下角，3：右上角，4：右下角，5：居中）：')
    )
    opacity = float(input('水印透明度（0-1）：'))
    processor = WatermarkProcessor(watermark_text, position_flag, opacity)
    results = process_path(image_path, processor)
    for success, message in results:
        print(message)
    if len(results) > 1:
        print('批量处理完成')


if __name__ == "__main__":
    main()
