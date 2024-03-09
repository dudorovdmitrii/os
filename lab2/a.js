setTimeout(() => {
  console.log("after timeout");
}, 3000);

// class A {
//   constructor(name) {
//     this.name = name;
//   }

//   clone() {
//     return new A(this.name);
//   }
// }

// class B extends A {
//   constructor(name, age) {
//     super(name);
//     this.age = age;
//   }

//   clone() {
//     return new B(this.name, this.age);
//   }
// }

// const b = new B("afs", 4);
// const c = b.clone();

// console.log(b, c);
