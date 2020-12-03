import shutil
import random
import json
import os


dataset_path = "data/test1"
path_class_names = os.path.join(dataset_path, "Dataset32a6aec8-8797-4785-af76-43e1ff8d92fe/annotation_definitions.json")
path_bounding_boxes = os.path.join(dataset_path, "Dataset32a6aec8-8797-4785-af76-43e1ff8d92fe/captures_000.json")
new_yolo_dataset_path = "data/yolo_dataset"

size_x, size_y = 1061, 564


# load class names
with open(path_class_names, "r") as json_file:
    data = json.load(json_file)

class_names = []
for element in data['annotation_definitions'][0]['spec']:
    class_names.append(element['label_name'])

print("num of classes:", len(class_names))
print("class names:", class_names)


# load labels
with open(path_bounding_boxes, "r") as json_file:
    data = json.load(json_file)

img_paths = []
bounding_boxes = {}
for element in data['captures']:

    filename = element['filename']
    img_paths.append(filename)

    bounding_boxes[filename] = []
    for bbox in element['annotations'][0]['values']:
        tmp = [bbox['label_id'], bbox['x'], bbox['y'], bbox['width'], bbox['height']]
        bounding_boxes[filename].append(tmp)

print("num of images:", len(img_paths))
print("image paths:", img_paths)


# create dataset folder
new_yolo_dataset_img_path = new_yolo_dataset_path + "/images"
if not os.path.exists(new_yolo_dataset_img_path):
    os.makedirs(new_yolo_dataset_img_path)

new_yolo_dataset_labels_path = new_yolo_dataset_path + "/labels"
if not os.path.exists(new_yolo_dataset_labels_path):
    os.makedirs(new_yolo_dataset_labels_path)


# generate class name info file
with open(os.path.join(new_yolo_dataset_path, "classes.names"), "w") as class_names_file:
    for class_name in class_names:
        class_names_file.write(class_name + '\n')


# copy images to new folder and for each of them generate txt file with bounding box info
new_img_paths = []
i = 0
for path in img_paths:
    img_name = str(i) + ".png"
    txt_name = str(i) + ".txt"
    
    src_path = dataset_path + '/' + path
    dst_path_img = new_yolo_dataset_img_path + '/' + img_name
    dst_path_txt = new_yolo_dataset_labels_path + '/' + txt_name

    shutil.copy(src_path, dst_path_img)
    new_img_paths.append(dst_path_img)

    with open(dst_path_txt, "w") as txt_file:
        for bbox in bounding_boxes[path]:
            label = str(bbox[0] - 1) + " " + str(bbox[1] / size_x) + " " + str(bbox[2] / size_y) \
                    + " " + str(bbox[3] / size_x) + " " + str(bbox[4] / size_y)
            txt_file.write(label + "\n")

    i += 1


# generate train/test info files
split_ratio = 0.9
random.shuffle(new_img_paths)
split_point = int(split_ratio * len(new_img_paths))
train_set = new_img_paths[:split_point]
test_set = new_img_paths[split_point:]
print("Train set length:", len(train_set))
print("Test set length:", len(test_set))

with open(new_yolo_dataset_path + "/train_set.txt", "w") as train_set_file:
    for filename in train_set:
        train_set_file.write(filename + '\n')

with open(new_yolo_dataset_path + "/test_set.txt", "w") as test_set_file:
    for filename in test_set:
        test_set_file.write(filename + '\n')
