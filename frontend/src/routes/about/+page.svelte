<script lang="ts">
	import { base } from '$app/paths';
</script>

<svelte:head>
	<title>About | BikeTitans</title>
</svelte:head>

<section>
	<h1 class="mb-6 text-center text-4xl font-bold">About Us</h1>

	<div class="mb-8 rounded-lg bg-white p-8 shadow-md">
		<h2 class="mb-4 text-2xl font-bold">Our Mission</h2>
		<p class="mb-6 text-gray-700">
			At BikeTitans, we're dedicated to providing real-time occupancy data to help travelers find
			safe and open bike rack locations for their bicycles and scooters.
		</p>

		<h2 class="mb-4 text-2xl font-bold">Our Story</h2>
		<p class="mb-6 text-gray-700">
			Founded in 2025, BikeTitans began with a simple idea: to create a solution that would help
			travelers store their bikes. Since then, we've grown into a team of dedicated professionals
			committed to excellence in data collection and analysis.
		</p>

		<div class="grid grid-cols-1 gap-6 md:grid-cols-3">
			<div class="rounded-lg bg-blue-50 p-6">
				<h3 class="mb-2 text-xl font-bold">Innovation</h3>
				<p>We constantly push the boundaries of occupancy tracking technology.</p>
			</div>

			<div class="rounded-lg bg-green-50 p-6">
				<h3 class="mb-2 text-xl font-bold">Accuracy</h3>
				<p>Our systems provide precise, reliable data you can trust.</p>
			</div>

			<div class="rounded-lg bg-amber-50 p-6">
				<h3 class="mb-2 text-xl font-bold">Customer Focus</h3>
				<p>We design our solutions with our users' needs at the forefront.</p>
			</div>
		</div>
	</div>

	<div class="rounded-lg bg-white p-8 shadow-md">
		<h2 class="mb-4 text-2xl font-bold">Our Team</h2>

		<div class="grid grid-cols-1 gap-8 md:grid-cols-2 lg:grid-cols-3">
			<div class="text-center">
				<img
					src="{base}/images/guy1.jpg"
					alt="Jane Doe"
					class="mx-auto mb-4 h-32 w-32 rounded-full object-cover"
				/>
				<h3 class="text-xl font-bold">Samyak Kakatur</h3>
				<p class="text-gray-600">CTO<br />Senior Product Manager<br />Senior Software Engineer</p>
			</div>

			<div class="text-center">
				<img
					src="{base}/images/guy2.jpg"
					alt="John Smith"
					class="mx-auto mb-4 h-32 w-32 rounded-full object-cover"
				/>
				<h3 class="text-xl font-bold">Joseph Caraan</h3>
				<p class="text-gray-600">Senior Software Engineer</p>
			</div>

			<div class="text-center">
				<img
					src="{base}/images/guy3.jpg"
					alt="Emily Johnson"
					class="mx-auto mb-4 h-32 w-32 rounded-full object-cover"
				/>
				<h3 class="text-xl font-bold">Jyro Jimenez</h3>
				<p class="text-gray-600">Senior Embedded Software Engineer</p>
			</div>
		</div>
	</div>
</section>
