This Silabs DLL has a built in filter that consists of the following flow:-

FM Radio ---> Default DirectSound Device

This will be used by default. This graph stuff is pure geekiness. Don't even touch unless you're a certified geek.

If you want to do something clever, you can use GraphEdit to create your own filter graph. Save and copy to the same directory as the dll & name it "filter.grf". 

Alternatively, you can choose from one of the predefined graphs included in the distribution.

Radio-DirectSound.grf = This is an external version of the built in graph, mostly for documentation purposes
Radio-FFDShow-DirectSound.grf = This one inserts an FFDShow Sound Processor in the filter chain. Obviously you must have FFDShow installed before this will work
Radio-DirectSound-VISTA.grf = This is REQUIRED if you use Windows Vista.
