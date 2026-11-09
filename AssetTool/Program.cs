using CodeWalker.GameFiles;
class Program {
 static void Add(RpfDirectoryEntry dest,string source){foreach(var f in Directory.GetFiles(source))RpfFile.CreateFile(dest,Path.GetFileName(f),File.ReadAllBytes(f));foreach(var d in Directory.GetDirectories(source))Add(RpfFile.CreateDirectory(dest,Path.GetFileName(d)),d);}
 static IEnumerable<RpfFile> Walk(RpfFile f){yield return f;if(f.Children!=null)foreach(var c in f.Children)foreach(var r in Walk(c))yield return r;}
 static void Check(Skeleton sk){var bs=sk.Bones.Items;if(bs.Length!=98)throw new Exception("Unexpected rig");for(int i=0;i<bs.Length;i++){if(bs[i].Index!=i||bs[i].Index2!=i)throw new Exception("Invalid bone index");var seen=new HashSet<int>();int n=i;while(n>=0){if(n>=bs.Length||!seen.Add(n))throw new Exception("Invalid ancestry");n=bs[n].ParentIndex;}}if(bs.Select(b=>b.Tag).Distinct().Count()!=bs.Length)throw new Exception("Duplicate bone tag");}
 static void Main(string[] a){try{Run(a);}catch(Exception e){Console.Error.WriteLine(e);Environment.ExitCode=1;}}
 static void Run(string[] a){
  if(a[0]=="model-profile"){
   var y=new YdrFile();y.Load(File.ReadAllBytes(a[1]));var points=new List<SharpDX.Vector3>();
   foreach(var model in y.Drawable.DrawableModels.High)foreach(var g in model.Geometries){var v=g.VertexData;int po=v.Info.GetComponentOffset(0);for(int i=0;i<v.VertexCount;i++){int o=i*v.VertexStride+po;points.Add(new SharpDX.Vector3(BitConverter.ToSingle(v.VertexBytes,o),BitConverter.ToSingle(v.VertexBytes,o+4),BitConverter.ToSingle(v.VertexBytes,o+8)));}}
   float lo=points.Min(p=>p.Y),hi=points.Max(p=>p.Y);Console.WriteLine("bounds "+y.Drawable.BoundingBoxMin+" / "+y.Drawable.BoundingBoxMax);
   for(int i=0;i<10;i++){var bin=points.Where(p=>p.Y>=lo+(hi-lo)*i/10&&p.Y<=lo+(hi-lo)*(i+1)/10).ToArray();if(bin.Length>0)Console.WriteLine($"y={lo+(hi-lo)*(i+.5f)/10:F3} maxZ={bin.Max(p=>p.Z):F3} minZ={bin.Min(p=>p.Z):F3}");}
  }else if(a[0]=="skin-audit"){
   var y=new YddFile();y.Load(File.ReadAllBytes(a[1]+".ydd"));var f=new YftFile();f.Load(File.ReadAllBytes(a[1]+".yft"));Check(f.Fragment.Drawable.Skeleton);int vertices=0,weighted=0;
   foreach(var d in y.DrawableDict.Drawables.data_items){
    // Component drawables without a private skeleton inherit the actor rig.
    // The old single-drawable audit incorrectly dereferenced these nulls.
    var sk=d.Skeleton??f.Fragment.Drawable.Skeleton;Check(sk);int n=sk.Bones.Items.Length;
    foreach(var model in new[]{d.DrawableModels.High,d.DrawableModels.Med,d.DrawableModels.Low,d.DrawableModels.VLow}.Where(m=>m!=null).SelectMany(m=>m)){
     if(model.BoneIndex>=n)throw new Exception("Model bone out of range");
     foreach(var g in model.Geometries){
      var v=g.VertexData;if(v==null)continue;
      var palette=g.BoneIds?.Length>0?g.BoneIds:Enumerable.Range(0,n).Select(i=>(ushort)i).ToArray();
      if(palette.Any(i=>i>=n))throw new Exception("Geometry palette out of range");
      int io=v.Info.GetComponentOffset(2),wo=v.Info.GetComponentOffset(1),po=v.Info.GetComponentOffset(0);
      if(v.VertexBytes.Length<(long)v.VertexCount*v.VertexStride)throw new Exception("Truncated vertices");
      foreach(var index in g.IndexBuffer?.Indices??Array.Empty<ushort>())if(index>=v.VertexCount)throw new Exception("Triangle index out of range");
      for(int i=0;i<v.VertexCount;i++){
       int start=i*v.VertexStride;
       if(po>=0)for(int axis=0;axis<3;axis++){float value=BitConverter.ToSingle(v.VertexBytes,start+po+axis*4);if(!float.IsFinite(value)||Math.Abs(value)>100)throw new Exception("Nonfinite/unbounded mesh position");}
       if(io>=0&&wo>=0)for(int j=0;j<4;j++)if(v.VertexBytes[start+wo+j]>0){if(v.VertexBytes[start+io+j]>=palette.Length)throw new Exception("Weighted GPU palette index out of range");weighted++;}
       vertices++;
      }
     }
    }
   }
   Console.WriteLine("PASS: "+vertices+" LOD vertices, "+weighted+" skin influences; actor/inherited rigs, GPU palettes/indices, triangles and finite positions");
  }else if(a[0]=="ped"){
   string stem=a[1],output=a[4];Directory.CreateDirectory(Path.GetDirectoryName(output));
   var y=new YddFile();y.Load(File.ReadAllBytes(stem+".ydd"));var sk=y.DrawableDict.Drawables.data_items.Single(d=>d.Skeleton!=null).Skeleton;Check(sk);
   var f=new YftFile();f.Load(File.ReadAllBytes(stem+".yft"));Check(f.Fragment.Drawable.Skeleton);
   if(!sk.Bones.Items.Select(b=>b.Tag).SequenceEqual(f.Fragment.Drawable.Skeleton.Bones.Items.Select(b=>b.Tag)))throw new Exception("Skeleton mismatch");
   f.Fragment.Name="pack:/"+a[3];File.Copy(stem+".ydd",output+".ydd",true);File.WriteAllBytes(output+".yft",f.Save());File.Copy(a[2],output+".ymt",true);
   var m=new YmtFile();m.Load(File.ReadAllBytes(a[2]));var tex=new YtdFile();tex.Load(File.ReadAllBytes(stem+".ytd"));
   var required=new HashSet<string>(StringComparer.OrdinalIgnoreCase);
   foreach(var drawable in y.DrawableDict.Drawables.data_items){
    if((drawable.ShaderGroup.TextureDictionary?.Textures?.Count??0)!=0)throw new Exception("Unexpected embedded textures: review redistribution first");
    foreach(var shader in drawable.ShaderGroup.Shaders.data_items)foreach(var p in shader.ParametersList.Parameters)if(p.DataType==0&&p.Data is TextureBase texture&&!string.IsNullOrWhiteSpace(texture.Name))required.Add(texture.Name);
   }
   var textures=tex.TextureDict.Textures.data_items.Where(t=>required.Contains(t.Name)).ToList();
   if(textures.Count!=required.Count)throw new Exception("Unresolved texture references");
   tex.TextureDict.BuildFromTextureList(textures);File.WriteAllBytes(output+".ytd",tex.Save());
   Console.WriteLine(a[3]+": 98 bones, "+y.DrawableDict.Drawables.Count+" drawables, only "+textures.Count+" used custom textures; "+string.Join(",",required));
  }else if(a[0]=="ycd"){
   var y=XmlYcd.GetYcd(File.ReadAllText(a[1]));File.WriteAllBytes(a[2],y.Save());var b=new YcdFile();RpfFile.LoadResourceFile(b,File.ReadAllBytes(a[2]),46);
   int samples=0;foreach(var pair in y.AnimMap){var x=pair.Value.Animation;var z=b.AnimMap[pair.Key].Animation;
    if(x.Frames!=z.Frames||x.Duration!=z.Duration)throw new Exception("Face timing changed");
    foreach(var id in z.BoneIds.data_items)if(!(id.BoneId>=62000&&id.BoneId<62112)&&!(id.BoneId>=63000&&id.BoneId<63010))throw new Exception("Unexpected body track "+id.BoneId);
    for(int i=0;i<x.BoneIds.data_items.Length;i++)for(int frame=0;frame<x.Frames;frame++){var pos=new Animation.FramePosition{Frame0=frame,Frame1=frame,Alpha0=1,Alpha1=0};if(x.BoneIds.data_items[i].Track==0){var delta=x.EvaluateVector4(pos,i,false)-z.EvaluateVector4(pos,i,false);if(float.IsNaN(delta.Length())||delta.Length()>1e-6)throw new Exception("Face position changed");}else if(x.BoneIds.data_items[i].Track==1){var delta=x.EvaluateQuaternion(pos,i,false)-z.EvaluateQuaternion(pos,i,false);if(float.IsNaN(delta.Length())||delta.Length()>1e-6)throw new Exception("Face rotation changed");}samples++;}
   }Console.WriteLine("Verified "+b.ClipMap.Count+" clips, "+samples+" face/tail samples; no body tracks");
  }else if(a[0]=="pack"){
   var p=Path.GetFullPath(a[2]);if(Path.GetExtension(p)!=".rpf")throw new Exception("Expected generated RPF target");var parent=Path.GetDirectoryName(p);Directory.CreateDirectory(parent);var temporary=Path.Combine(parent,"build-"+Guid.NewGuid().ToString("N")+".rpf");var r=RpfFile.CreateNew(parent,Path.GetFileName(temporary));Add(r.Root,a[1]);File.Move(temporary,p,true);Console.WriteLine("Packed "+p);
  }else if(a[0]=="audit"){
   var r=new RpfFile(Path.GetFullPath(a[1]),Path.GetFileName(a[1]));r.ScanStructure(s=>{},s=>throw new Exception(s));int count=0;
   foreach(var archive in Walk(r))foreach(var e in archive.AllEntries.OfType<RpfFileEntry>()){
    if(e.Name.EndsWith(".rpf"))continue;
    var data=archive.ExtractFile(e);string relative=e.Path[(r.Name.Length+1)..].Replace('\\','/');string source=Path.Combine(a[2],relative.Replace('/',Path.DirectorySeparatorChar));
    // Nested RPF contents are compared against the explicit ped source folder.
    if(relative.Contains("cuavatars.rpf/"))source=Path.Combine(a[3],e.Name);
    if(!File.Exists(source)||!data.SequenceEqual(RawResource(File.ReadAllBytes(source))))throw new Exception("Readback differs: "+e.Path);
    count++;
   }Console.WriteLine("PASS: "+count+" packed entries match build inputs");
  }
 }
 static byte[] RawResource(byte[] bytes){if(bytes.Length<16||BitConverter.ToUInt32(bytes,0)!=0x37435352)return bytes;using var input=new MemoryStream(bytes,16,bytes.Length-16);using var zip=new System.IO.Compression.DeflateStream(input,System.IO.Compression.CompressionMode.Decompress);using var output=new MemoryStream();zip.CopyTo(output);return output.ToArray();}
}
