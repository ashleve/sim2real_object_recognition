using System;
using UnityEngine;
using UnityEngine.Experimental.Perception.Randomization.Parameters;
using UnityEngine.Experimental.Perception.Randomization.Randomizers;
using UnityEngine.Experimental.Perception.Randomization.Samplers;

[Serializable]
[AddRandomizerMenu("Perception/My Light Randomizer")]
public class MyLightRandomizer : Randomizer
{
    public FloatParameter lightIntensityParameter;
    public ColorRgbParameter lightColorParameter;
    public Vector3Parameter rotation = new Vector3Parameter
        {
            x = new UniformSampler(0, 360),
            y = new UniformSampler(0, 360),
            z = new UniformSampler(0, 360)
        };

    protected override void OnIterationStart()
    {
        var taggedObjects = tagManager.Query<MyLightRandomizerTag>();

        foreach (var taggedObject in taggedObjects)
        {
            var light = taggedObject.GetComponent<Light>();
            var direction = taggedObject.GetComponent<Transform>();

            taggedObject.transform.rotation = Quaternion.Euler(rotation.Sample());
            light.intensity = lightIntensityParameter.Sample(); 
            light.color = lightColorParameter.Sample();           
        }
    }
}
