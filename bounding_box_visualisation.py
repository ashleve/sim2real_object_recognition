import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from matplotlib import patches


path = "data/yolo_dataset/train_set.txt"

size_x, size_y = 1061, 564

with open(path, "r") as file:
    img_path = file.readline().replace("\n", "")
    txt_path = path.rsplit('/', 1)[0] + "/labels/" + (img_path.rsplit('.', 1)[0] + '.txt').rsplit('/', 1)[-1]

with open(txt_path, "r") as file:
    bboxes = file.readlines()


print(img_path)
print(txt_path)

bounding_boxes = []
for bbox in bboxes:
    bbox_data = bbox.replace("\n", "").split(" ")
    bbox_data = [float(n) for n in bbox_data]
    bounding_boxes.append(bbox_data)
    print(bbox_data)


fig, ax = plt.subplots()
fig.set_size_inches(15, 10)
img = mpimg.imread(img_path)
ax.imshow(img)

for bbox in bounding_boxes:
    rect = patches.Rectangle(
        (bbox[1] * size_x, bbox[2] * size_y),
        bbox[3] * size_x,
        bbox[4] * size_y,
        linewidth=2,
        edgecolor='r',
        facecolor='none'
    )
    ax.add_patch(rect)

plt.show()

fig.savefig("bbox_example.png")
